#include "miniz.h"
#include "unziplib.h"
#include <pthread.h>
#include <direct.h>
#include <dirent.h>
#include <string.h>

struct data {
    char* zip_file_path;
    char* extract_folder;
};

struct data_2 {
	mz_zip_archive* zip_archive;
	char* extract_folder;
	int index;
};

void* unzip_file_thread(void* arg);

void* unzip_thread(void* arg){

    struct data* pd = (struct data*)arg;

    char* zip_file_path = pd->zip_file_path;
    char* extract_folder = pd->extract_folder;

    _mkdir(extract_folder);

	printf("Extracting : %s\n", zip_file_path);

	mz_zip_archive zip_archive;
	memset(&zip_archive, 0, sizeof(zip_archive));

	//zip読み込み
	mz_zip_reader_init_file(&zip_archive, zip_file_path, 0);

	//アーカイブ内のファイル数を取得
	mz_uint max_index = mz_zip_reader_get_num_files(&zip_archive);


	//Multi-Thread
    pthread_t thread_array[max_index];
    
    struct data_2 data_array[max_index];

	for(mz_uint index = 0; index < max_index; index++){

		//Multi-Thread
    	data_array[index].zip_archive = &zip_archive;
		data_array[index].extract_folder = extract_folder;
    	data_array[index].index = index;

		//解凍開始
    	pthread_create(&thread_array[index], NULL, unzip_file_thread, &data_array[index]);
	}

	for(mz_uint index = 0; index < max_index; index++){
		//全スレッドの終了を待機
		pthread_join(thread_array[index], NULL);
	}

    printf("Complete : %s\n", zip_file_path);

	//メモリ解放
	mz_zip_reader_end(&zip_archive);
    free(zip_file_path);
    free(extract_folder);

    return NULL;
}


void* unzip_file_thread(void* arg){

    struct data_2* pd = (struct data_2*)arg;

	mz_zip_archive* zip_archive = pd->zip_archive;
	char* extract_folder = pd->extract_folder;
	int index = pd->index;

	//ディレクトリであるかどうか
	mz_bool isdir = mz_zip_reader_is_file_a_directory(zip_archive, index);

	//ファイル名のバッファサイズ
	mz_uint buf_size = mz_zip_reader_get_filename(zip_archive, index, "", 0);

	//ファイル名バッファ
	char filename[buf_size];

	//ファイル名を取得
	mz_zip_reader_get_filename(zip_archive, index, filename, buf_size);


	if(!isdir){
		//ファイル解凍
		char path[128];

		sprintf(path, "%s/%s", extract_folder, filename);

		mz_zip_reader_extract_to_file(zip_archive, index, path, 0);
	}else{
		//フォルダ作成
		char path[128];

		sprintf(path, "%s/%s", extract_folder, filename);

		_mkdir(path);
	}

	return NULL;
}