#include "miniz.c"
#include "miniz.h"
#include "unziplib.h"
#include "unzipthread.c"
#include <direct.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


void unzip(char* folder_path, char* extract_folder_path){

	if(strlen(extract_folder_path) != 0){
		printf("Cleaning %s folder...\n", extract_folder_path);

		delete_files(extract_folder_path);
		delete_dirs(extract_folder_path);
	}

	printf("Creating %s folder...\n", extract_folder_path);

	_mkdir(extract_folder_path);

	DIR *dir;
	struct dirent *dp;
	dir=opendir(folder_path);

	char filenames[128][128] = {0,};
	int index = 0;

	for(dp=readdir(dir); dp!=NULL; dp=readdir(dir)){
		//.zipが名前に含まれるかどうか
		if(strstr(dp->d_name, ".zip") != NULL){
			if(strlen(dp->d_name) <= 128){
				strcpy(filenames[index], dp->d_name);
				index++;
			}
		}
	}

	closedir(dir);


	//Multi-Thread
    pthread_t thread_array[index+1];
    
    struct data data_array[index+1];

	for(int i = 0; i < index; i++){

		char* filename = filenames[i];

		//zipファイルのパス
		char* file_path = (char *)malloc(sizeof(char) * 128);

		sprintf(file_path, "%s/%s", folder_path, filename);

		//ファイルのタイトル
		char title[128] = {0,};

		getFileTitle(title, filename);

		//展開先フォルダのパス
		char* extract_folder = (char *)malloc(sizeof(char) * 128);

		sprintf(extract_folder, "%s/%s", extract_folder_path, title);

		//Multi-Thread
    	data_array[i].zip_file_path = file_path;
    	data_array[i].extract_folder = extract_folder;

		//解凍開始
    	pthread_create(&thread_array[i], NULL, unzip_thread, &data_array[i]);
	}

	for(int i = 0; i < index; i++){
		//全スレッドの終了を待機
		pthread_join(thread_array[i], NULL);
	}

	printf("ALL DONE!\n");
}


void getFileTitle(char* title, char* filename){

	//.zipが出てくるアドレスを取得
	char* p = strstr(filename, ".zip");

	//.zipが出てくるまでの名前をtitleにコピー
	for(char* pi = filename; pi <= p; pi++){
		int index = (int)(pi - filename);
		if(pi != p)
			title[index] = filename[index];
		else
			title[index] = '\0';
	}
}


void delete_files(char *dir)
{
    DIR *dp;
    struct dirent *ent;
    struct stat statbuf;

    if ((dp = opendir(dir)) == NULL) {
        perror(dir);
		return;
    }
    chdir(dir);
    while ((ent = readdir(dp)) != NULL) {
        stat(ent->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(".", ent->d_name) == 0 ||
                strcmp("..", ent->d_name) == 0)
                continue;
            delete_files(ent->d_name);
        }
        else {
            unlink(ent->d_name);
        }
    }
    chdir("..");
    closedir(dp);
}


void delete_dirs(char *dir)
{
    DIR *dp;
    struct dirent *ent;
    struct stat statbuf;

    if ((dp = opendir(dir)) == NULL) {
        perror(dir);
        return;
    }
    chdir(dir);
    while ((ent = readdir(dp)) != NULL) {
        stat(ent->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(".", ent->d_name) == 0 ||
                strcmp("..", ent->d_name) == 0)
                continue;
            if (rmdir(ent->d_name) < 0) {
                delete_dirs(ent->d_name);
            }
        }
    }
    chdir("..");
    closedir(dp);

    if (rmdir(dir) < 0) {
        perror(dir);
        return;
    }
}


void main() {

	char* zipfolderpath = "zips";
	char* folderpath = "worlds";

	clock_t start_clock, end_clock;
	start_clock = clock();

	unzip(zipfolderpath, folderpath);

	end_clock = clock();

	printf(
        "Time : %fms\n", 
        (double)(end_clock - start_clock)
    );
}