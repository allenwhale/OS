#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
#define cluster_mask 0x0FFFFFFF
#define first_sector_of_cluster(cluster, x) ((((cluster) - 2) * x->sectors_per_cluster) + first_data_sector)
#define is_dir(dir) (dir->attribute & 0x10)
#define is_hidden(dir) (dir->attribute & 0x02)
#define first_cluster(dir) (dir->high_first_cluster << 16 | dir->low_first_cluster)
#define end_cluster(cluster) ((cluster & cluster_mask) >= 0x0FFFFFF8)
#define bad_cluster(cluster) ((cluster & cluster_mask) == 0x0FFFFFF7)
#define unused_cluster(cluster) ((cluster & cluster_mask) == 0)
#define ok_cluster(cluster) (0x00000002 <= (cluster & cluster_mask) && (cluster & cluster_mask) <= 0x0FFFFFEF)
#define LFN_flag 0x0F
#define deleted_entry_flag 0xe5
#define dot_entry_flag 0x2e
typedef struct fat_extBS_32{
    unsigned int        table_size_32;
    unsigned short      extended_flags;
    unsigned short      fat_version;
    unsigned int        root_cluster;
    unsigned short      fat_info;
    unsigned short      backup_BS_sector;
    unsigned char       reserved_0[12];
    unsigned char       drive_number;
    unsigned char       reserved_1;
    unsigned char       boot_signature;
    unsigned int        volume_id;
    unsigned char       volume_label[11];
    unsigned char       fat_type_label[8];
}__attribute__((packed)) fat_extBS_32_t;
typedef struct fat_BS{
    unsigned char       bootjmp[3];
    unsigned char       oem_name[8];
    unsigned short          bytes_per_sector;
    unsigned char       sectors_per_cluster;
    unsigned short      reserved_sector_count;
    unsigned char       table_count;
    unsigned short      root_entry_count;
    unsigned short      total_sectors_16;
    unsigned char       media_type;
    unsigned short      table_size_16;
    unsigned short      sectors_per_track;
    unsigned short      head_side_count;
    unsigned int        hidden_sector_count;
    unsigned int        total_sectors_32;
    unsigned char       ext_section[54];
}__attribute__((packed)) fat_BS_t;
typedef struct dir_table{
    unsigned char       filename[11];
    unsigned char       attribute;
    unsigned char       reversed;
    unsigned char       creation_second;
    unsigned short      creation_time;
    unsigned short      creation_date;
    unsigned short      last_accessed_date;
    unsigned short      high_first_cluster;
    unsigned short      last_modified_time;
    unsigned short      last_modified_date;
    unsigned short      low_first_cluster;
    unsigned int        size_of_file;
}__attribute__((packed)) dir_table_t;
typedef struct LFN{
    unsigned char       seq_number;
    unsigned short      filename1[5];
    unsigned char       attribute;
    unsigned char       type;
    unsigned char       checksum;
    unsigned short      filename2[6];
    unsigned short      reversed;
    unsigned short      filename3[2];
}__attribute__((packed)) LFN_t;

fat_BS_t *fat_BS;
fat_extBS_32_t *fat_ext_BS;
unsigned int *fat_table;
unsigned int root_dir_sectors, first_data_sector, fat_size, first_fat_sector, data_sectors, total_clusters, first_root_dir_sector, root_cluster_32, first_sector_of_cluster, entry_per_sector, bytes_per_cluster;
vector<dir_table_t*> recovery_file;
set<string> used_filename;
int read_BS(int);
int read_fat(int);
int read_sector(int, unsigned char *, unsigned int);
int read_cluster(int, unsigned int, const string &, string&);
int read_cluster_chain(int, unsigned int, const string &);

int read_BS(int fd){
    fat_BS = new fat_BS_t;
    unsigned char buf[512];
    if(read(fd, buf, 512) <= 0){
        perror("read BS failed");
        return -1;
    }
    memcpy(fat_BS, buf, sizeof(fat_BS_t));
    fat_ext_BS = (fat_extBS_32_t*)fat_BS->ext_section;
    fat_size = (fat_BS->table_size_16 == 0) ? fat_ext_BS->table_size_32 : fat_BS->table_size_16;
    root_dir_sectors = ((fat_BS->root_entry_count * 32) + (fat_BS->bytes_per_sector - 1)) / fat_BS->bytes_per_sector;
    first_data_sector = fat_BS->reserved_sector_count + (fat_BS->table_count * fat_size) + root_dir_sectors;
    first_fat_sector = fat_BS->reserved_sector_count;
    data_sectors = fat_BS->total_sectors_32 - (fat_BS->reserved_sector_count + (fat_BS->table_count * fat_size) + root_dir_sectors);
    total_clusters = data_sectors / fat_BS->sectors_per_cluster;
    first_root_dir_sector = first_data_sector - root_dir_sectors;
    root_cluster_32 = fat_ext_BS->root_cluster;
    entry_per_sector = fat_BS->bytes_per_sector / 32;
    bytes_per_cluster = fat_BS->bytes_per_sector * fat_BS->sectors_per_cluster;
    return 0;
}

int read_fat(int fd){
    fat_table = new unsigned int [fat_size * fat_BS->bytes_per_sector / 4];
    //printf("%u\n", first_fat_sector * fat_BS->bytes_per_sector);
    if(lseek(fd, first_fat_sector * fat_BS->bytes_per_sector, SEEK_SET) < 0){
        perror("lseek failed");
        return -1;
    }
    if(read(fd, (unsigned char*)fat_table, fat_size * fat_BS->bytes_per_sector) <= 0){
        perror("read fat failed");
        return -1;
    }
    return 0;
}
int read_sector(int fd, unsigned char *sector, unsigned int sector_num){
    if(lseek(fd, sector_num * fat_BS->bytes_per_sector, SEEK_SET) < 0){
        perror("lseek failed");
        return -1;
    }
    if(read(fd, sector, fat_BS->bytes_per_sector) <= 0){
        perror("read sector failed");
        return -1;
    }
    return 0;
}
int cnt = 0;
string filename8_3(const char *buf){
    string filename = "";
    string ext = "";
    for(int i=0;i<8;i++){
        if(buf[i] != ' ')
            filename += buf[i];
        else break;
    }
    for(int i=8;i<11;i++){
        if(buf[i] != ' ')
            ext += buf[i];
        else break;
    }
    return (ext == "") ? filename : (filename + "." + ext);

}

int read_cluster(int fd, unsigned int cluster, const string &now_filename, string &lfn_filename){
    unsigned int first_sector = first_sector_of_cluster(cluster, fat_BS);
    unsigned char *sector = new unsigned char [fat_BS->bytes_per_sector];
    int end = false;
    int lfn_count = 0;
    for(int i=0;i<fat_BS->sectors_per_cluster&&!end;i++){
        if(read_sector(fd, sector, first_sector + i) < 0)
            return -1;
        for(int j=0;j<(int)entry_per_sector;j++){
            unsigned char *entry = (sector + (32 * j));
            if(entry[0] == 0){
                end = true;
                lfn_filename = "";
                break;
            }
            if(entry[0] == deleted_entry_flag){
                if(lfn_filename == ""){
                    for(int k=0;k<26;k++){
                        entry[0] = 'A' + k;
                        if(used_filename.find(string(entry, entry+11)) == used_filename.end()){
                            break;
                        }
                    }
                }else{
                    entry[0] = toupper(lfn_filename[0]);
                }
                used_filename.insert(string(entry, entry + 11));
                int offset = ((first_sector + i) * fat_BS->bytes_per_sector + (32 * j));
                lseek(fd, offset, SEEK_SET);
                write(fd, entry, 1);
            }
            if(entry[0] == dot_entry_flag)
                continue;
            if(entry[11] == LFN_flag){ // LFN entry
                lfn_count++;
                LFN_t *lfn = (LFN_t*)entry;
                string filename = "";
                bool filename_end =  false;
                for(int k=0;k<5&&!filename_end;k++){
                    if(lfn->filename1[k]) filename += (char)lfn->filename1[k];
                    else filename_end = true;
                }
                for(int k=0;k<6&&!filename_end;k++){
                    if(lfn->filename2[k]) filename += (char)lfn->filename2[k];
                    else filename_end = true;
                }
                for(int k=0;k<2&&!filename_end;k++){
                    if(lfn->filename3[k]) filename += (char)lfn->filename3[k];
                    else filename_end = true;
                }
                lfn_filename = filename + lfn_filename;
            }else{ //dir table entry
                dir_table_t *dir_table = (dir_table_t*)entry;
                for(int k=1;k<=lfn_count;k++){
                    int offset = ((first_sector + i) * fat_BS->bytes_per_sector + (32 * (j - k)));
                    lseek(fd, offset, SEEK_SET);
                    unsigned char buf = k;
                    if(k == lfn_count) buf += 0x40;
                    write(fd, &buf, 1);
                }
                lfn_count = 0;
                if(is_hidden(dir_table)){
                    lfn_filename = "";
                    continue;
                }
                string filename = (lfn_filename == "") ? filename8_3((char*)dir_table->filename) : lfn_filename;
                if(is_dir(dir_table)){ // directory
                    printf("%s\n", (now_filename + filename + "/").c_str());
                    read_cluster_chain(fd, first_cluster(dir_table), (now_filename + filename + "/"));
                    lfn_filename = "";
                }else{ // file
                    cnt ++;
                    printf("%s\n", (now_filename + filename).c_str());
                    dir_table_t *new_dir_table = new dir_table_t;
                    memcpy(new_dir_table, dir_table, sizeof(dir_table_t));
                    recovery_file.push_back(new_dir_table);
                    lfn_filename = "";
                }
            }
        }
    }
    return 0;
}

int read_cluster_chain(int fd, unsigned int cluster, const string &now_filename){
    string lfn_filename = "";
    unsigned int pre = 0;
    while(ok_cluster(cluster)){
        read_cluster(fd, cluster, now_filename, lfn_filename);
        pre = cluster;
        cluster = fat_table[cluster] & 0x0FFFFFFF;
    }
    fat_table[pre] = 0x0FFFFFFF;
    lseek(fd, first_fat_sector * fat_BS->bytes_per_sector + pre * 4, SEEK_SET);
    unsigned int buf = 0x0FFFFFFF;
    write(fd, (unsigned char*)&buf, 4);
    return 0;
}
int write_fat_table(int fd, unsigned int cluster, unsigned int val){
    int offset = first_fat_sector * fat_BS->bytes_per_sector + cluster * 4;
    lseek(fd, offset, SEEK_SET);
    return write(fd, &val, 4);
}
int recovery_cluster_chain(int fd){
    vector<dir_table_t*> n_recovery_file;
    vector<bool> done(recovery_file.size(), false);
    vector<unsigned int> cluster(recovery_file.size(), 0);
    vector<unsigned int> pre_cluster(recovery_file.size(), 0);
    for(int i=0;i<(int)recovery_file.size();i++){
        dir_table_t *dir_table = recovery_file[i];    
        cluster[i] = first_cluster(dir_table);
        dir_table->size_of_file -= min(dir_table->size_of_file, bytes_per_cluster);
    }
    while(true){
        for(int i=0;i<(int)recovery_file.size();i++){
            if(done[i] == true)continue;
            dir_table_t *dir_table = recovery_file[i];
            if(dir_table->size_of_file == 0){ // write end into fat_table[cluster]
                fat_table[cluster[i]] = 0x0FFFFFFF;
                write_fat_table(fd, cluster[i], fat_table[cluster[i]]);
                done[i] = true;
            }else{ // find next unused cluster and write into fat_table[origin cluster]
                unsigned int new_cluster = cluster[i] + 1;
                while(!unused_cluster(fat_table[new_cluster])) new_cluster++;
                fat_table[cluster[i]] = new_cluster;
                write_fat_table(fd, cluster[i], fat_table[cluster[i]]);
                cluster[i] = new_cluster;
                dir_table->size_of_file -= min(dir_table->size_of_file, bytes_per_cluster);
            }
        }
        int end = true;
        for(bool tf: done)
            end &= tf;
        if(end) break;
    }
    return 0;
}

int recovery(const string &img){
    string lfn_filename = "";
    int fd = open(img.c_str(), O_RDWR );
    if(fd <= 0){
        perror("open failed");
        return -1;
    }
    if(read_BS(fd) < 0){
        return -1;
    }
    if(read_fat(fd) < 0){
        return -1;
    }
    read_cluster_chain(fd, root_cluster_32, "./");
    for(auto dir_table: recovery_file){
        printf("waiting for recovery: %s %u %u\n", dir_table->filename, first_cluster(dir_table), dir_table->size_of_file);
    }
    recovery_cluster_chain(fd);
    close(fd);
    return 0;
}

int main(int argc, char **argv){
    if(argc < 2){
        printf("USAGE: ./listall DEVICE");
        return 0;
    }
    recovery(argv[1]);
    return 0;
}
