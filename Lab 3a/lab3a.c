// NAME: Justin Hong, Kedar Thiagarajan
// EMAIL: justinjh@ucla.edu, kedarbt@ucla.edu
// ID: 604565186, 504539433

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include "ext2_fs.h"

#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_SIZE 1024
#define GROUP_DESCRIPTOR_TABLE_SIZE 32
#define INODE_SIZE 128

int file_fd;
struct superblock_t *super_summary;
struct group_t *group_summaries;
int num_groups;
int* validInodeAddresses;
int* validInodeNums;
int numValidInodes = 0;

struct superblock_t
{
	uint16_t inode_size;
	uint32_t block_num, inode_num, block_size, blocks_per_group, inodes_per_group, first_inode;
};

struct group_t
{
	uint16_t group_num, num_blocks, num_free_blocks, num_free_inodes;
	uint32_t num_inodes, free_block_bitmap_blocknum, free_inode_bitmap_blocknum, first_inode_blocknum;
};

void processSuperblock()
{
	pread(file_fd, &(super_summary->block_num), 4, SUPERBLOCK_OFFSET + 4);
	pread(file_fd, &(super_summary->inode_num), 4, SUPERBLOCK_OFFSET);
	pread(file_fd, &(super_summary->block_size), 4, SUPERBLOCK_OFFSET + 24);
	super_summary->block_size = EXT2_MIN_BLOCK_SIZE << super_summary->block_size;
	pread(file_fd, &(super_summary->inode_size), 2, SUPERBLOCK_OFFSET + 88);
	pread(file_fd, &(super_summary->blocks_per_group), 4, SUPERBLOCK_OFFSET + 32);
	pread(file_fd, &(super_summary->inodes_per_group), 4, SUPERBLOCK_OFFSET + 40);
	pread(file_fd, &(super_summary->first_inode), 4, SUPERBLOCK_OFFSET + 84);
	fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", super_summary->block_num, super_summary->inode_num, 
		super_summary->block_size, super_summary->inode_size, super_summary->blocks_per_group, 
		super_summary->inodes_per_group, super_summary->first_inode);
}

void processGroups()
{
	int total_blocks = super_summary->block_num;
	int total_inodes = super_summary->inode_num;
    num_groups = ceil((double) super_summary->block_num/super_summary->blocks_per_group);
    group_summaries = malloc(num_groups * sizeof(struct group_t));
    int i;
    for (i = 0; i < num_groups; i++)
    {
        group_summaries[i].group_num = i;

        if (total_blocks >= super_summary->blocks_per_group)
        {
        	total_blocks -= super_summary->blocks_per_group;
        	group_summaries[i].num_blocks = super_summary->blocks_per_group;
        }
        else
        	group_summaries[i].num_blocks = total_blocks;

        if (total_inodes >= super_summary->inodes_per_group)
        {
        	total_inodes -= super_summary->inodes_per_group;
        	group_summaries[i].num_inodes = super_summary->inodes_per_group;
        }
        else
        	group_summaries[i].num_inodes = total_inodes;

        // count number of free blocks
        pread(file_fd, &(group_summaries[i].num_free_blocks), 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i*GROUP_DESCRIPTOR_TABLE_SIZE) + 12);
        // count number of free inodes
        pread(file_fd, &(group_summaries[i].num_free_inodes), 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i*GROUP_DESCRIPTOR_TABLE_SIZE) + 14);
        // free block bitmap block number
        pread(file_fd, &(group_summaries[i].free_block_bitmap_blocknum), 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i*GROUP_DESCRIPTOR_TABLE_SIZE));
        //free inode bitmap block number
        pread(file_fd, &(group_summaries[i].free_inode_bitmap_blocknum), 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i*GROUP_DESCRIPTOR_TABLE_SIZE) + 4);
        //first inode blocknum
        pread(file_fd, &(group_summaries[i].first_inode_blocknum), 2, SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE + (i*GROUP_DESCRIPTOR_TABLE_SIZE) + 8);
       	fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", group_summaries[i].group_num, group_summaries[i].num_blocks, 
		group_summaries[i].num_inodes, group_summaries[i].num_free_blocks, group_summaries[i].num_free_inodes, 
		group_summaries[i].free_block_bitmap_blocknum, group_summaries[i].free_inode_bitmap_blocknum, group_summaries[i].first_inode_blocknum);
    }
}

void processBlockBitmap()
{
	int i;
	for (i = 0; i < num_groups; i++)
	{
		int group_bytes;
		int extra = group_summaries[i].num_blocks%8;
		if (extra != 0)
			group_bytes = (group_summaries[i].num_blocks/8)+1;
		else
			group_bytes = group_summaries[i].num_blocks/8;
		int j;
		for (j = 0; j < group_bytes; j++)
		{
			uint8_t byte;
			pread(file_fd, &byte, 1, (group_summaries[i].free_block_bitmap_blocknum*super_summary->block_size)+j);
			uint8_t mask = 1;
			int k;
			for (k = 1; k <= 8; k++)
			{
				if (j == group_bytes-1 && k > extra && extra != 0)
					break;
				if (!(byte & mask))
					fprintf(stdout, "BFREE,%d\n", (8*j)+(i*group_summaries[i].num_blocks)+k);
				mask = mask << 1;
			}
		}
	}
}

void processInodeBitmap()
{
	int i;
	for (i = 0; i < num_groups; i++)
	{
		int group_bytes;
		int extra = group_summaries[i].num_inodes%8;
		if (extra != 0)
			group_bytes = (group_summaries[i].num_inodes/8)+1;
		else
			group_bytes = group_summaries[i].num_inodes/8;
		int j;
		for (j = 0; j < group_bytes; j++)
		{
			uint8_t byte;
			pread(file_fd, &byte, 1, (group_summaries[i].free_inode_bitmap_blocknum*super_summary->block_size)+j);
			uint8_t mask = 1;
			int k;
			for (k = 1; k <= 8; k++)
			{
				if (j == group_bytes-1 && k > extra && extra != 0)
					break;
				if (!(byte & mask))
					fprintf(stdout, "IFREE,%d\n", (8*j)+(i*group_summaries[i].num_inodes)+k);
				mask = mask << 1;
			}
		}
	}
}

void processDirectories(int* directory_info)
{
	int correct_location = directory_info[0];
	int inodeNumber = directory_info[1];
    int j;
    for (j = 0; j < 12; j++) // parse the direct block addresses to get info
    {
    	uint32_t block_id;
        pread(file_fd, &block_id, 4, correct_location + 40 + (j*4)); //offset of block addresses = 40, each address is 4 bytes
        if (block_id != 0)
        {  
        	int parent_inode_number;
            uint8_t name_length;
            uint16_t entry_length, byte_offset;
            uint32_t inode_number;
            int currOffset = super_summary->block_size * block_id;
            byte_offset = 0;
            while (currOffset < (super_summary->block_size*block_id + super_summary->block_size))
            {
                //get parent inode number
                parent_inode_number = inodeNumber;

                //get inode number of the referenced file
                pread(file_fd, &inode_number, 4, currOffset);

                //get entry length
                pread(file_fd, &entry_length, 2, currOffset + 4);

                if (inode_number == 0) //something wrong: inode number 0 is reserved (starts with 1)
                {
                    currOffset += entry_length;
                    byte_offset += entry_length;
                    continue;
                }

                //get name length
                pread(file_fd, &name_length, 1, currOffset + 6);

                fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,", parent_inode_number, byte_offset, inode_number, entry_length, name_length);
                
                char char_buf;
                fprintf(stdout, "'");
                int k;
                for (k = 0; k < name_length; k++)
                {
                    pread(file_fd, &char_buf, 1, currOffset + 8 + k);
                    fprintf(stdout, "%c", char_buf);
                }
                fprintf(stdout, "'\n");
                
                byte_offset += entry_length;
                currOffset += entry_length;
            } 
        }
    }

    //check indirect block
    uint32_t block_id;
    pread(file_fd, &block_id, 4, correct_location + 40 + (12*4));
    if (block_id != 0)
    {
    	int i;
    	for (i = 0; i < (super_summary->block_size/4); i++)
    	{
    		int currOffset = super_summary->block_size*block_id + (i * 4);
    		uint32_t block2_id;
    		pread(file_fd, &block2_id, 4, currOffset);
    		if (block2_id != 0) //check for validity
    		{
    			int parent_inode_number;
            	uint8_t name_length;
            	uint16_t entry_length, byte_offset;
           	 	uint32_t inode_number;
            	byte_offset = 0;
    			currOffset = block2_id*super_summary->block_size;
    			while (currOffset < (block2_id * super_summary->block_size + super_summary->block_size))
    			{
    				//parent inode number
    				parent_inode_number = inodeNumber;
    				//get name length
    				pread(file_fd, &name_length, 1, currOffset + 6);
    				//get inode number
    				pread(file_fd, &inode_number, 4, currOffset);
    				//get entry length
    				pread(file_fd, &entry_length, 2, currOffset + 4);
    				if (inode_number == 0) //something wrong: inode number 0 is reserved (starts with 1)
                	{
                    	currOffset += entry_length;
                    	byte_offset += entry_length;
                    	continue;
                	}

                	fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,", parent_inode_number, byte_offset, inode_number, entry_length, name_length);
                	char char_buf;
               	 	fprintf(stdout, "'");
                	int k;
                	for (k = 0; k < name_length; k++)
                	{
                	    pread(file_fd, &char_buf, 1, currOffset + 8 + k);
                	    fprintf(stdout, "%c", char_buf);
                	}
                	fprintf(stdout, "'\n");
                	//get byte offset
                	byte_offset += entry_length;
                	currOffset += entry_length;
    			}
    		}
    	}
    }

    pread(file_fd, &block_id, 4, correct_location + 40 + (13*4));
    if (block_id != 0)
    {
    	int i;
    	for (i = 0; i < (super_summary->block_size/4); i++)
    	{
    		int currOffset = super_summary->block_size*block_id + (i * 4);
    		uint32_t block2_id;
    		pread(file_fd, &block2_id, 4, currOffset);
    		if (block2_id != 0) //check for validity
    		{
    			int k;
    			for (k = 0; k < super_summary->block_size/4; k++)
    			{
    				uint32_t block3_id;
    				pread(file_fd, &block3_id, 4, block2_id*super_summary->block_size + (k * 4));
    				if (block3_id != 0)
    				{
		    			int parent_inode_number;
		            	uint8_t name_length;
		            	uint16_t entry_length, byte_offset;
		           	 	uint32_t inode_number;
		            	byte_offset = 0;
		    			currOffset = block3_id*super_summary->block_size;
		    			while (currOffset < (block3_id * super_summary->block_size + super_summary->block_size))
		    			{
		    				//parent inode number
		    				parent_inode_number = inodeNumber;
		    				//get name length
		    				pread(file_fd, &name_length, 1, currOffset + 6);
		    				//get inode number
		    				pread(file_fd, &inode_number, 4, currOffset);
		    				//get entry length
		    				pread(file_fd, &entry_length, 2, currOffset + 4);
		    				if (inode_number == 0) //something wrong: inode number 0 is reserved (starts with 1)
		                	{
		                    	currOffset += entry_length;
		                    	byte_offset += entry_length;
		                    	continue;
		                	}

		                	fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,", parent_inode_number, byte_offset, inode_number, entry_length, name_length);
		                	pread(file_fd, &name_length, 1, currOffset + 6);
		                	char char_buf;
		               	 	fprintf(stdout, "'");
		                	int k;
		                	for (k = 0; k < name_length; k++)
		                	{
		                	    pread(file_fd, &char_buf, 1, currOffset + 8 + k);
		                	    fprintf(stdout, "%c", char_buf);
		                	}
		                	fprintf(stdout, "'\n");
		                	//get byte offset
		                	byte_offset += entry_length;
		                	currOffset += entry_length;
		    			}
		    		}
		    	}
    		}
    	}
    }

    pread(file_fd, &block_id, 4, correct_location + 40 + (14*4));
    if (block_id != 0)
    {
    	int i;
    	for (i = 0; i < (super_summary->block_size/4); i++)
    	{
    		int currOffset = super_summary->block_size*block_id + (i * 4);
    		uint32_t block2_id;
    		pread(file_fd, &block2_id, 4, currOffset);
    		if (block2_id != 0) //check for validity
    		{
    			int k;
    			for (k = 0; k < super_summary->block_size/4; k++)
    			{
    				uint32_t block3_id;
    				pread(file_fd, &block3_id, 4, block2_id*super_summary->block_size + (k * 4));
    				if (block3_id != 0)
    				{
    					int j;
    					for (j = 0; j < super_summary->block_size/4; j++)
    					{
    						uint32_t block4_id;
    						pread(file_fd, &block4_id, 4, block3_id*super_summary->block_size + (k * 4));
    						if (block4_id != 0)
    						{
    							int parent_inode_number;
				            	uint8_t name_length;
				            	uint16_t entry_length, byte_offset;
				           	 	uint32_t inode_number;
				            	byte_offset = 0;
				    			currOffset = block4_id*super_summary->block_size;
				    			while (currOffset < (block4_id * super_summary->block_size + super_summary->block_size))
				    			{
				    				//parent inode number
				    				parent_inode_number = inodeNumber;
				    				//get name length
				    				pread(file_fd, &name_length, 1, currOffset + 6);
				    				//get inode number
				    				pread(file_fd, &inode_number, 4, currOffset);
				    				//get entry length
				    				pread(file_fd, &entry_length, 2, currOffset + 4);
				    				if (inode_number == 0) //something wrong: inode number 0 is reserved (starts with 1)
				                	{
				                    	currOffset += entry_length;
				                    	byte_offset += entry_length;
				                    	continue;
				                	}

				                	fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,", parent_inode_number, byte_offset, inode_number, entry_length, name_length);
				                	pread(file_fd, &name_length, 1, currOffset + 6);
				                	char char_buf;
				               	 	fprintf(stdout, "'");
				                	int k;
				                	for (k = 0; k < name_length; k++)
				                	{
				                	    pread(file_fd, &char_buf, 1, currOffset + 8 + k);
				                	    fprintf(stdout, "%c", char_buf);
				                	}
				                	fprintf(stdout, "'\n");
				                	//get byte offset
				                	byte_offset += entry_length;
				                	currOffset += entry_length;
				    			}
    						}
    					}
    				}
    			}
    		}
    	}
    }
}

void processIndirectBlockRefs()
{
    int i;
    for (i = 0; i < numValidInodes; i++)
    {
        uint32_t blocknum;
        pread(file_fd, &blocknum, 4, validInodeAddresses[i] + 40 + (12*4)); // single indirect node
        int j;
        int byte_offset = 12;
        for (j = 0; j < (super_summary->block_size/4); j++)
        {
            uint32_t blocknum_2; 
            pread(file_fd, &blocknum_2, 4, blocknum*super_summary->block_size + (j*4));
            if (blocknum_2 != 0) //obtain info if valid block
            {
                fprintf(stdout, "INDIRECT,%d,1,%d,%d,%d\n", validInodeNums[i], byte_offset, blocknum, blocknum_2);
            }
            byte_offset++;
        }
        //double indirect
        pread(file_fd, &blocknum, 4, validInodeAddresses[i] + 40 + (13*4));
        byte_offset = 268;
        for (j = 0; j < (super_summary->block_size/4); j++)
        {
        	uint32_t blocknum_2;
        	pread(file_fd, &blocknum_2, 4, blocknum*super_summary->block_size + (j*4));
        	if (blocknum_2 != 0)
        	{
        		fprintf(stdout, "INDIRECT,%d,2,%d,%d,%d\n", validInodeNums[i], byte_offset, blocknum, blocknum_2);
        		int k;
                for (k = 0; k < (super_summary->block_size/4); k++)
                {
                	uint32_t blocknum_3;
                    pread(file_fd, &blocknum_3, 4, blocknum_2 * super_summary->block_size + (k*4));
                    if (blocknum_3 != 0)
                    {
                        fprintf(stdout, "INDIRECT,%d,1,%d,%d,%d\n",validInodeNums[i], byte_offset, blocknum_2, blocknum_3);
                    }
                }
        	}
        	byte_offset += 256;
        }
 		//triple indirect
        pread(file_fd, &blocknum, 4, validInodeAddresses[i] + 40 + (14*4));
        byte_offset = 65804;
        for (j = 0; j < (super_summary->block_size/4); j++)
        {
        	uint32_t blocknum_2;
            pread(file_fd, &blocknum_2, 4, blocknum*super_summary->block_size + (j*4));
            if (blocknum_2 != 0)
            {
                fprintf(stdout, "INDIRECT,%d,3,%d,%d,%d\n",validInodeNums[i], byte_offset, blocknum, blocknum_2);
                int k;
                for (k = 0; k < (super_summary->block_size/4); k++)
                {
                	uint32_t blocknum_3;
                    pread(file_fd, &blocknum_3, 4, blocknum_2 * super_summary->block_size + (k*4));
                    if (blocknum_3 != 0)
                    {
                        fprintf(stdout, "INDIRECT,%d,2,%d,%d,%d\n",validInodeNums[i], byte_offset, blocknum_2, blocknum_3);
                        int l;
                        for (l = 0; l < (super_summary->block_size/4); l++)
                        {
                        	uint32_t blocknum_4;
                        	pread(file_fd, &blocknum_4, 4, blocknum_3*super_summary->block_size + (l*4));
                        	if(blocknum_4 != 0)
                        	{
                        		fprintf(stdout, "INDIRECT,%d,1,%d,%d,%d\n",validInodeNums[i], byte_offset, blocknum_3, blocknum_4);
                        	}
                        }
                    }
                }
            }
            byte_offset += 65536;
        }
    }
}

void processInode()
{
	int validDirectories[2];
    int i, inodeNumber;
    int directory_found = 0;
    char* file_type;
    char creation_time[64], mod_time[64], access_time[64];
    uint16_t link_count, file_mode, file_num, owner;
    uint32_t group_of_inode, group_of_inode2, number_of_blocks;
    uint64_t file_size, file_size2;
    validInodeAddresses = malloc(super_summary->inode_num * sizeof(int));
    validInodeNums = malloc(super_summary->inode_num * sizeof(int));
 
    for (i = 0; i < num_groups; i++)
    {
    	int group_bytes;
		int extra = group_summaries[i].num_inodes%8;
		if (extra != 0)
			group_bytes = (group_summaries[i].num_inodes/8)+1;
		else
			group_bytes = group_summaries[i].num_inodes/8;
        int j;
        for (j = 0; j < group_bytes; j++)
        {
        	uint8_t byte;
            pread(file_fd, &byte, 1, (group_summaries[i].free_inode_bitmap_blocknum * super_summary->block_size) + j);
            uint8_t mask = 1;
            int bit;
            for (bit = 1; bit <= 8; bit++)
            {
            	if (j == group_bytes-1 && bit > extra && extra != 0)
					break;
                int correct_location = j*8 + bit;
                if ((byte & mask)) // is the inode free
                {
                    // find inode number
                    inodeNumber = correct_location + (i * group_summaries[i].num_inodes);
 
                    // find file type
                    pread(file_fd, &file_num, 2, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location-1)*INODE_SIZE);
 					validInodeAddresses[numValidInodes] = group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location-1)*INODE_SIZE;
					validInodeNums[numValidInodes] = inodeNumber;
					numValidInodes++;
                    if (file_num & 0x8000)
                        file_type = "f";
                    else if (file_num & 0xA000)
                        file_type = "s";
                    else if (file_num & 0x4000)
                    {
                        file_type = "d";
                        validDirectories[0] = group_summaries[i].first_inode_blocknum* super_summary->block_size + (correct_location-1)*INODE_SIZE;
                        validDirectories[1] = inodeNumber;
                        directory_found = 1;
                    }
                    else
                        file_type = "?";
 
                    //find file mode
                    file_mode = file_num & 0xFFF; //print this in OCTAL
                    if (file_mode == 0)
                    	continue;
 
                    //get owner
                    pread(file_fd, &owner, 2, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 2);
                    
                    //find group
                    pread(file_fd, &group_of_inode, 2, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 24);
                    pread(file_fd, &group_of_inode2, 2, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 122);
                    group_of_inode |= (group_of_inode2 << 16);
 
                    //find link count
                    pread(file_fd, &link_count, 2, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 26);
                    if (link_count == 0)
                    	continue;
 
                    //find creation time
                    uint32_t time_num;
                    pread(file_fd, &time_num, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 12);
                    struct tm ts;
                    time_t time = time_num;
                    ts = *gmtime(&time);
                    strftime(creation_time, sizeof(creation_time), "%m/%d/%y %H:%M:%S", &ts);

                    pread(file_fd, &time_num, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 16);
 					time = time_num;
 					ts = *gmtime(&time);
                    strftime(mod_time, sizeof(mod_time), "%m/%d/%y %H:%M:%S", &ts);

                    pread(file_fd, &time_num, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 8);
 					time = time_num;
 					ts = *gmtime(&time);
                    strftime(access_time, sizeof(access_time), "%m/%d/%y %H:%M:%S", &ts);
                    
                    //get file size
                    pread(file_fd, &file_size, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 4);
                    pread(file_fd, &file_size2, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 108);
                    file_size |= (file_size2 << 32);
 
                    //get number of blocks
                    pread(file_fd, &number_of_blocks, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 28);

					fprintf(stdout, "INODE,%d,%s,%o,%d,%d,%d,%s,%s,%s,%ld,%d,", inodeNumber, file_type, file_mode, owner, group_of_inode, link_count, 
						creation_time, mod_time, access_time, file_size, number_of_blocks);
					int k;
					uint32_t buf;
                    for (k = 0; k < 15; k++)
                    {
                    	pread(file_fd, &buf, 4, group_summaries[i].first_inode_blocknum * super_summary->block_size + (correct_location - 1)*INODE_SIZE + 40 + (k*4));
                    	if (k == 14)
                    		fprintf(stdout, "%d\n", buf);
                    	else
                    		fprintf(stdout, "%d,", buf);
                    }
                    if (directory_found)
                    	processDirectories(validDirectories);
                    directory_found = 0;
                }
                mask = mask << 1;
            }
        }
    }
 
}

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s fileSystemImage\n", argv[0]);
		exit(1);
	}
	file_fd = open(argv[1], O_RDONLY);
	if (file_fd == -1)
	{
		fprintf(stderr, "Error opening file system image: %s\n", strerror(errno));
		exit(2);
	}
	super_summary = malloc(sizeof(struct superblock_t));
	processSuperblock();
	processGroups();
	processBlockBitmap();
	processInodeBitmap();
	processInode();
	processIndirectBlockRefs();
}