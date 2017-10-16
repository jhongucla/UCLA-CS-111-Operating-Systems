# NAME: Justin Hong, Kedar Thiagarajan
# EMAIL: justinjh@ucla.edu, kedarbt@ucla.edu
# ID: 604565186, 504539433

#!/usr/bin/python

import sys, string, locale
allocate_errors = []

def get_fs_info():
    block_bitmap = []
    inode_bitmap = []
    inode_table_start = 0
    num_blocks = 0
    block_size = 0
    num_inodes = 0
    inodes_per_group = 0
    free_blocks = []
    free_inodes = []
    file.seek(0)
    for line in file:
        if line.startswith("SUPERBLOCK"):
            superblock_line = line.rstrip('\n').split(',')
            inodes_per_group = int(superblock_line[6])
            num_blocks = int(superblock_line[1])
            num_inodes = int(superblock_line[2])
            block_size = int(superblock_line[3])   
        if line.startswith("GROUP"):
            group_line = line.rstrip('\n').split(',')
            block_bitmap.append(group_line[6])
            inode_bitmap.append(group_line[7])
            inode_table_start = int(group_line[8])
        if line.startswith("BFREE"):
            bfree_line = line.rstrip('\n').split(',')
            free_blocks.append(int(bfree_line[1]))
        if line.startswith("IFREE"):
            ifree_line = line.rstrip('\n').split(',')
            free_inodes.append(int(ifree_line[1]))
    return (free_blocks, free_inodes, block_bitmap, inode_bitmap, block_size, num_blocks, num_inodes, inodes_per_group, inode_table_start)

def check_directories():
    global allocate_errors
    inode_info = get_fs_info()
    free_inodes = inode_info[1]
    num_inodes = inode_info[6]
    file.seek(0)
    linerinos = file.readlines()
    file.seek(0)
    for line in file:
        if line.startswith("DIRENT"):
            dirent_line = line.rstrip('\n').split(',')
            parent_inode = int(dirent_line[1])
            ref_inode = int(dirent_line[3])
            name = dirent_line[6]
            if (ref_inode < 0 or ref_inode > num_inodes):
                print("DIRECTORY INODE " + str(parent_inode) + " NAME " + name + " INVALID INODE " + str(ref_inode))
            elif (ref_inode in free_inodes and ref_inode not in allocate_errors):
                print("DIRECTORY INODE " + str(parent_inode) + " NAME " + name + " UNALLOCATED INODE " + str(ref_inode))
            if (name == "'.'" and parent_inode != ref_inode):
                print("DIRECTORY INODE " + str(parent_inode) + " NAME '.' LINK TO INODE " + str(ref_inode) + " SHOULD BE " + str(parent_inode))
            if (name == "'..'"):
                found_parent = parent_inode
                for line in linerinos:
                    if line.startswith("DIRENT"):
                        check_line = line.rstrip('\n').split(',')
                        check_ref = int(check_line[3])
                        check_parent = int(check_line[1])
                        check_name = check_line[6]
                        if (check_parent != parent_inode and check_ref == parent_inode and check_name != "'..'"):
                            found_parent = check_parent
                if (ref_inode != found_parent):
                    print("DIRECTORY INODE " + str(parent_inode) + " NAME '..' LINK TO INODE " + str(ref_inode) + " SHOULD BE " + str(found_parent))

#check for unallocated inodes 
def check_inodes():
    global allocate_errors
    inode_info = get_fs_info()
    free_inodes = inode_info[1]
    num_inodes = inode_info[6]
    unallocated = {}
    global inode_allocation_info
    inode_allocation_info = []
    file.seek(0)
    for line in file:
        if line.startswith("INODE"):
            inode_line = line.split(',')
            mode = int(inode_line[3])
            if(mode == 0 and int(inode_line[1]) not in free_inodes):
                print("UNALLOCATED INODE " + str(inode_line[1]) + " NOT ON FREELIST")
            elif(mode != 0):
                inode_allocation_info.append(int(inode_line[1]))
    for inode_num in inode_allocation_info:
        if(inode_num in free_inodes):
            print("ALLOCATED INODE " + str(inode_num) + " ON FREELIST")
            allocate_errors.append(int(inode_num))
    for x in range (11,num_inodes):
        if(x not in free_inodes and x not in inode_allocation_info):
            print("UNALLOCATED INODE " + str(x) + " NOT ON FREELIST")


# go through all blocks and make sure they are valid
def blockConsistency():
    block_info = get_fs_info()
    free_blocks = block_info[0]
    block_size = block_info[4]
    num_blocks = block_info[5]
    num_inodes = block_info[6]
    inode_table_start = block_info[8]
    allocated_blocks = set()
    referenced_blocks = {}
    file.seek(0)
    for line in file:
        if line.startswith("INODE"):
            inode_line = line.rstrip('\n').split(',')
            data_block_pointers = inode_line[12:]
            counter = 0 # count which data block you are looking at
            for block_pointer in data_block_pointers:
                int_pointer = int(block_pointer)
                if (counter < 12):
                    #direct pointers
                    if(int_pointer > num_blocks or int_pointer < 0):
                        print("INVALID BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET " + str(counter))
                    if(int_pointer in range(1, (inode_table_start + num_inodes/8))):
                        print("RESERVED BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET " + str(counter))
                    if (int_pointer != 0 and int_pointer in referenced_blocks):
                        print_line = referenced_blocks[int_pointer].split(',')
                        print("DUPLICATE " + print_line[2] + str(int_pointer) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])
                        referenced_blocks[int_pointer] = str(inode_line[1]) + "," + str(counter) + ",BLOCK ,1"
                    if(int_pointer != 0 and int_pointer not in referenced_blocks):
                        referenced_blocks[int_pointer] = str(inode_line[1]) + "," + str(counter) + ",BLOCK ,0"
                elif (counter == 12):
                    #indirect inode
                    if(int_pointer > num_blocks or int_pointer < 0):
                        print("INVALID INDIRECT BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET 12")
                    if(int_pointer in range(1, (inode_table_start + num_inodes/8))):
                        print("RESERVED INDIRECT BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET 12")
                    if (int_pointer != 0 and int_pointer in referenced_blocks):
                        print_line = referenced_blocks[int_pointer].split(',')
                        print("DUPLICATE " + print_line[2] + str(int_pointer) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])   
                        referenced_blocks[int_pointer] = str(inode_line[1]) + ",12" + ",INDIRECT BLOCK ,1"
                    if (int_pointer != 0 and int_pointer not in referenced_blocks):
                        referenced_blocks[int_pointer] = str(inode_line[1]) + ",12" + ",INDIRECT BLOCK ,0"  
                elif (counter == 13):
                    #double indirect block
                    if(int_pointer > num_blocks or int_pointer < 0):
                        print("INVALID DOUBLE INDIRECT BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET 268")
                    if(int_pointer in range(1, (inode_table_start + num_inodes/8))):
                        print("RESERVED DOUBLE INDIRECT BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET 268")
                    if (int_pointer != 0 and int_pointer in referenced_blocks):
                        print_line = referenced_blocks[int_pointer].split(',')
                        print("DUPLICATE " + print_line[2] + str(int_pointer) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1]) 
                        referenced_blocks[int_pointer] = str(inode_line[1]) + ",268" + ",DOUBLE INDIRECT BLOCK ,1"
                    if(int_pointer != 0 and int_pointer not in referenced_blocks):
                        referenced_blocks[int_pointer] = str(inode_line[1]) + ",268" + ",DOUBLE INDIRECT BLOCK ,0"
                elif (counter == 14):
                    #triple indirect block
                    if(int_pointer > num_blocks or int_pointer < 0):
                        print("INVALID TRIPPLE INDIRECT BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET 65804")
                    if(int_pointer in range(1, (inode_table_start + num_inodes/8))):
                        print("RESERVED TRIPPLE INDIRECT BLOCK " + block_pointer + " IN INODE " + inode_line[1] + " AT OFFSET 65804")
                    if (int_pointer != 0 and int_pointer in referenced_blocks):
                        print_line = referenced_blocks[int_pointer].split(',')
                        print("DUPLICATE " + print_line[2] + str(int_pointer) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])   
                        referenced_blocks[int_pointer] = str(inode_line[1]) + ",65804" + ",TRIPPLE INDIRECT BLOCK ,1"
                    if(int_pointer != 0 and int_pointer not in referenced_blocks):
                        referenced_blocks[int_pointer] = str(inode_line[1]) + ",65804" + str(counter) + ",TRIPPLE INDIRECT BLOCK ,0"
                if(int_pointer in free_blocks):
                        allocated_blocks.add(int_pointer);
                counter += 1
        if line.startswith("INDIRECT"):
            indirect_line = line.rstrip('\n').split(',')
            indirect_block = int(indirect_line[4])
            ref_block = int(indirect_line[5])
            if(indirect_block in free_blocks):
                allocated_blocks.add(indirect_block);
            if(ref_block in free_blocks):
                allocated_blocks.add(ref_block);
            if(int(indirect_line[2]) == 3):
                if(indirect_block > num_blocks or indirect_block < 0):
                    print("INVALID TRIPPLE INDIRECT BLOCK " + str(indirect_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(indirect_block in range(1, (inode_table_start + num_inodes/8))):
                    print("RESERVED TRIPPLE INDIRECT BLOCK " + str(indirect_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(ref_block > num_blocks or ref_block < 0):
                    print("INVALID DOUBLE INDIRECT BLOCK " + str(ref_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(ref_block in range(1, (inode_table_start + num_inodes/8))):
                    print("RESERVED DOUBLE INDIRECT BLOCK " + str(ref_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if (ref_block != 0 and ref_block in referenced_blocks):
                    print_line = referenced_blocks[ref_block].split(',')
                    print("DUPLICATE " + print_line[2] + str(ref_block) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])   
                    referenced_blocks[ref_block] = indirect_line[1] + "," + indirect_line[3] + ",DOUBLE INDIRECT BLOCK ,1"
                if(ref_block != 0 and ref_block not in referenced_blocks):
                    referenced_blocks[ref_block] = indirect_line[1] + "," + indirect_line[3] + ",DOUBLE INDIRECT BLOCK ,0"
            elif(int(indirect_line[2]) == 2):
                if(indirect_block > num_blocks or indirect_block < 0):
                    print("INVALID DOUBLE INDIRECT BLOCK " + str(indirect_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(indirect_block in range(1, (inode_table_start + num_inodes/8))):
                    print("RESERVED DOUBLE INDIRECT BLOCK " + str(indirect_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(ref_block > num_blocks or ref_block < 0):
                    print("INVALID INDIRECT BLOCK " + str(ref_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(ref_block in range(1, (inode_table_start + num_inodes/8))):
                    print("RESERVED INDIRECT BLOCK " + str(ref_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if (ref_block != 0 and ref_block in referenced_blocks):
                    print_line = referenced_blocks[ref_block].split(',')
                    print("DUPLICATE " + print_line[2] + str(ref_block) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])   
                    referenced_blocks[ref_block] = indirect_line[1] + "," + indirect_line[3] + ",INDIRECT BLOCK ,1"
                if(ref_block != 0 and ref_block not in referenced_blocks):
                    referenced_blocks[ref_block] = indirect_line[1] + "," + indirect_line[3] + ",INDIRECT BLOCK ,0"
            elif(int(indirect_line[2]) == 1):
                if(indirect_block > num_blocks or indirect_block < 0):
                    print("INVALID INDIRECT BLOCK " + str(indirect_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(indirect_block in range(1, (inode_table_start + num_inodes/8))):
                    print("RESERVED INDIRECT BLOCK " + str(indirect_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(ref_block > num_blocks or ref_block < 0):
                    print("INVALID BLOCK " + str(ref_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if(ref_block in range(1, (inode_table_start + num_inodes/8))):
                    print("RESERVED BLOCK " + str(ref_block) + " IN INODE " + indirect_line[1] + " AT OFFSET " + indirect_line[3])
                if (ref_block != 0 and ref_block in referenced_blocks):
                    print_line = referenced_blocks[ref_block].split(',')
                    print("DUPLICATE " + print_line[2] + str(ref_block) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])   
                    referenced_blocks[ref_block] = indirect_line[1] + "," + indirect_line[3] + ",BLOCK ,1"
                if(ref_block != 0 and ref_block not in referenced_blocks):
                    referenced_blocks[ref_block] = indirect_line[1] + "," + indirect_line[3] + ",BLOCK ,0"
    for block in allocated_blocks:
        print("ALLOCATED BLOCK " + str(block) + " ON FREELIST")
    for key,line in referenced_blocks.items():
        print_line = line.split(',')
        if (int(print_line[3]) > 0):
            print("DUPLICATE " + print_line[2] + str(key) + " IN INODE " + print_line[0] + " AT OFFSET " + print_line[1])
    for x in range((inode_table_start + num_inodes/8), num_blocks):
        if (x not in free_blocks and x not in referenced_blocks):
            print("UNREFERENCED BLOCK " + str(x))
                                 
def verify_link_count():
    file.seek(0)
    lines = file.readlines()
    for line in lines:
        if line.startswith("INODE"):
            link_line = line.split(',')
            link_count = int(link_line[6])
            inode_num = int(link_line[1])
            check = 0
            for l in lines:
                if l.startswith("DIRENT"):
                    dir_line = l.split(',')
                    if int(dir_line[3]) == inode_num:
                        check += 1
            if (link_count != check):
                print("INODE " + str(inode_num) + " HAS " + str(check) + " LINKS BUT LINKCOUNT IS " + str(link_count))

def main():
    global file
    try:
        file = open(sys.argv[1], 'r')
    except:
        sys.stderr.write("Error opening files")
        sys.exit(1)
    blockConsistency()
    verify_link_count()
    check_inodes()
    check_directories()

if __name__ == "__main__":
    main()