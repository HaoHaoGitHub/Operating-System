/* Hao Chen  661412823  chenh15@rpi.edu */
/* Compile: gcc -Wall main.c -pthread */
/* Command line: ./a.out <directory> <substring> */
/* For example, if you want to complie it with your current directory, 
   you just need to type: ./a.out . <substring> */

#include <stdio.h> /* printf and file */
#include <stdlib.h> /* dynamic memory allocation */
#include <string.h> /* string */
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h> 

#define MAX_wordlength 50 
// global variables 
int size = 16; 
int num_txt = 0;

/* global mutex variable */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//--------------------------------------------------------------
/* Here I define two structures. The word_t is a structure that 
is used for storing the word's name and what file it comes from. 
The file_t is a structure a pointer to the array, filename, and 
index */
typedef struct word {
	char* word;
	char filename[80];
} word_t;

typedef struct file {
	char filename[80];
	int* index;
	word_t** ptr;
} file_t;
//--------------------------------------------------------------

/* Helper function for thread creation. */
void* operate_file(void* arg) {

	file_t file_system = *((file_t* )arg);
    printf("MAIN THREAD: Assigned \'%s\' to child thread %u.\n",file_system.filename, (unsigned int)pthread_self());

	FILE* file = NULL;
    //------------------------------------------------------------
	file = fopen(file_system.filename, "r");
    //------------------------------------------------------------
    if (file == NULL) {
        perror("Failed to open the file!\n");
        return NULL;
    } 

	char* temp = (char*) calloc (MAX_wordlength, sizeof(char));
    if (temp == NULL) {
        perror("Calloc for temp failed.");
        return NULL;
    }
    word_t word_system;

	while (fscanf(file, "%s", temp) == 1) { /* Scanning each word one by one */
    	word_system.word = (char*) calloc (strlen(temp) + 1, sizeof(char));
        if (word_system.word == NULL) {
            perror("Calloc failed.");
            return NULL;
        }
    	strcpy(word_system.word, temp);
    	strcpy(word_system.filename, file_system.filename);
    	/* Put the word struct into the Array, we need use mutex */
        /* Child threads are responsible for re-allocating memory, as necessary. Such operations
        must be synchronized. When a child thread adds a word, the only part that must be synchronized 
        is obtaining the designated index. When memory for the array needs to be re-allocated, only one 
        thread can do this. */
    	pthread_mutex_lock( &mutex );
        if (*(file_system.index) == size) {
            /* Start with an array size of 16. If the size of the array needs to be increased, 
            use realloc() to do so, doubling the size each time.t */
            size *= 2;
            *(file_system.ptr) = (word_t*) realloc (*(file_system.ptr), size * sizeof(word_t));
            if (*(file_system.ptr) == NULL) {
                perror("Failed to reallcated memory!\n");
                return NULL;
            } else {
                printf("THREAD %u: Re-allocated array of %d character pointers.\n", (unsigned int)pthread_self(), size);
            }
        }
        ((*(file_system.ptr))[*(file_system.index)])= word_system;
        printf("THREAD %u: Added \'%s\' at index %d.\n", (unsigned int)pthread_self(), word_system.word, 
               *(file_system.index));
  
        *(file_system.index) += 1;
    	pthread_mutex_unlock( &mutex );
	}
    
    free(temp);
    free(arg);
    fclose(file);
	return NULL;
}

/* Helper function to find the .txt file. */
int has_txt_extension(char const* name) {
	size_t len = strlen(name);
	if (len > 4 && strcmp(name + len - 4, ".txt") == 0) return 1;
	else return 0;
}

int main (int argc, char* argv[]) {
    
	if (argc != 3) {
		perror("Invalid arguments");
        perror("USAGE: ./a.out <directory> <substring>");
		return EXIT_FAILURE;
	}

    /* open the given directory */
    DIR* dir = opendir(argv[1]);
    if (dir == NULL) {
    	perror("opendir() failed");
    	return EXIT_FAILURE;
    }

    word_t* Array = NULL; 
    Array = (word_t*) calloc (size, sizeof(word_t));
    if (Array == NULL) {
    	perror("Calloc for Array failed.");
    	return EXIT_FAILURE;
    }
	printf("MAIN THREAD: Allocated initial array of 16 character pointers.\n");
    
    pthread_t tid[100]; /* # of txt files */
    struct dirent* file;
    int count_index = 0;
    while ((file = readdir(dir)) != NULL) {
        struct stat buf;
        int rc = lstat(file->d_name, &buf);
        if (rc == -1) {
            perror( "lstat() failed" );
            return EXIT_FAILURE;
        }

        if (S_ISREG(buf.st_mode)) {
            /* If the file is txt file */
            if (has_txt_extension(file->d_name)) {
                /* dynaically allocate memory for the child thread. */
                file_t * file_system = (file_t*) malloc ( sizeof(file_t) );
                if (file_system == NULL) {
                    perror("Calloc for file_t failed.");
                    return EXIT_FAILURE;
                }
                strcpy(file_system->filename, file->d_name);
                /* Here I use a pointer to keep track of the current index. */
                file_system->index = &count_index;
                /* Here I use a pointer to keep track of the outside Array. */
                file_system->ptr = &Array;

                int rc = pthread_create(&tid[num_txt], NULL, operate_file, file_system);
                if (rc != 0) {
                    fprintf( stderr, "pthread_create() failed (%d): %s\n", rc, strerror( rc ) );
                    return EXIT_FAILURE;
                } 
                num_txt++;
            }
        }
    }

    if (num_txt == 0) {
        perror("There is no .txt file in the current directory.");
        return EXIT_FAILURE;
    }

    //--------------------------------------------------------------------------
    int i;
    for (i = 0; i < num_txt; ++i) {
    	pthread_join(tid[i], NULL);
    }
    //--------------------------------------------------------------------------

    if (num_txt > 1) {
        printf("MAIN THREAD: All done (successfully read %d words from %d files).\n", count_index, num_txt);
    } else {
        printf("MAIN THREAD: All done (successfully read %d words from %d file).\n", count_index, num_txt);
    }

    //----------------------------------------------------------------------------
    //----------------------------------------------------------------------------
    printf("MAIN THREAD: Words containing substring \"%s\" are:\n", argv[2]);
    const char* substr = argv[2];
    int found_word = 0;
    for(i = 0; i < count_index;++i) {
        if (strstr(Array[i].word, substr) != NULL) {
            found_word = 1;
            printf("MAIN THREAD: %s (from \'%s\')\n", Array[i].word, Array[i].filename);
        }
    }
    if (!found_word) {
        perror("None.");
        perror("There is no such word!");
        return EXIT_FAILURE;
    }
    // ----------------------------------------------------------------------------
    closedir(dir);
    for (i = 0; i < count_index; ++i) {
        free(Array[i].word);
    }
    free(Array);
    Array = NULL;
    return EXIT_SUCCESS;
}



