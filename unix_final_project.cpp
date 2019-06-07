#include<iostream>
#include<iomanip>
#include<cstdlib>
#include<ctime>
#include<vector>
#include<fstream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<errno.h>
#include<string.h>
#include<pthread.h>

using namespace std;

const int dataSize = 50; // dataSize is size of struction named Data
const int tempSize = 100; // tempSize is size of input file

// Data is to hole data to be passed to a thread
struct Data
{
    pthread_t tid; // thread id
    string id; // document id
    string content; // original document content
    string str_array[100] = {" "}; // content after strtok
    vector< string > str;
    int data_id;
    int size = 0; // size of str_array
    int count; // count there are how many word in string
    int vec_size; // size of vector
    double coefficient[100]; // record every coefficient in array
    int double_index = 0; // size of  coefficient
    double average; // average of coefficient array
};

// share data
vector< Data >share_data;

// jaccard_function is to calculate jaccard similarity coefficient
void *jaccard_function(void *ptr);

// gettid function
pid_t gettid()
{
    return syscall(SYS_gettid);
}


int main(int argc, char *argv[]){
	//vector< Data > share_data;
    Data data[100] = {};
    int storeIndex = 0; // index of store
    clock_t start; // start time
    long double duration; // execution time
    start = clock();

    string s; // s read file and store into tempStore

    // ifstream constructor opens the file
    ifstream infile(argv[1], ios::in);
    // exit program if ifstream could not open file
    if (!infile)
    {
        cout << "File could not be opened!" << endl;
        return 0;
    }

    // store id and content from file
    while (getline(infile, s, '\n'))
    {
        //tempStore[storeIndex] = s;
        if (storeIndex % 2 == 0)
        {
            data[storeIndex / 2].id = s;
            storeIndex++;
        }
        else
        {
            data[storeIndex / 2].content = s;
            storeIndex++;
        }
    }

    for (int i = 0; i < storeIndex / 2; i++)
    {
        // assign data_id to data
        data[i].data_id  = i;

        // convert_char convert string to char
        // temp_char temporary store converted char
        char *convert_char, *temp_char;
        convert_char = new char[data[i].content.size() + 1];
        
         // copy string of data to convert_char
         strcpy(convert_char, data[i].content.c_str());
         // seperate string by space
         temp_char = strtok(convert_char, " ");
         while (temp_char != NULL)
         {
             data[i].str_array[data[i].size] = temp_char; 
             data[i].size++;
             temp_char = strtok(NULL, " ");
         }
    }

    // close file
    infile.close();
	
	
	for (int i = 0; i < storeIndex / 2; i++)
    {
        for(int j = 0; j < data[i].size; j++)
        {
            data[i].str.push_back(data[i].str_array[j]);
        }
    }

    // push data into vector
    for (int i = 0; i < storeIndex / 2; i++)
        share_data.push_back(data[i]);
    
    // delete the same word in str_array and adjust size of str_array
    for (unsigned int i = 0; i < share_data.size(); i++)
    {   
        vector< string >::reverse_iterator it = share_data[i].str.rbegin();
        for(int j = 0; j < share_data[i].size; j++)
        {
            vector< string >::reverse_iterator it1 = share_data[i].str.rbegin();
            it1 = it1 + share_data[i].str.size() - 2 - j;
            it = it1;
            for(int k = j+1; k < share_data[i].str.size(); k++)
            {
                if (share_data[i].str[j] == share_data[i].str[k])
                { 
                    vector< string >::iterator temp_it = it.base();
                    --temp_it;
                    share_data[i].str.erase(temp_it);

                }
                --it;
            }
        }
    }
    
    // set number of word in str
    for (int i = 0; i < storeIndex / 2; i++)
    {
        share_data[i].count = share_data[i].str.size();
        share_data[i].vec_size = share_data.size();
    }
    
    // create number of thread
    for(int i = 0; i < storeIndex / 2; i++)
    {
        // create thread for each data
        int check = pthread_create (&data[i].tid, NULL,
                jaccard_function, (void *)&share_data[i]);

        // check thread create success or fail
        if (check != 0)
        {
            cout << "\n\nThread create error!\n\n";
            exit(-1);
        }
        cout << "[Main thread]:create TID:" << data[i].tid
             << ",DocID:" << data[i].id << endl;
        sleep(1);
    }
    
    // join to terminated thread
    for(int i = 0; i < storeIndex/2; i++)
    {
        int check = pthread_join(data[i].tid, NULL);
        if (check != 0)
        {
            cout << "\n\nThread join error!\n\n";
            exit(-1);
        }
    }

    double max = share_data[0].average;
    string max_id = share_data[0].id;
    for (int i = 1; i < share_data.size(); i++)
    {
        if (share_data[i].average > max)
        {
            max_id = share_data[i].id;
            max = share_data[i].average;
        }
    }

    cout << "[Main thread] KeyDocID:" << max_id << " HighestJ:" << max << endl;
    
    for (int i = 0; i < share_data.size(); i++)
    {
        if(share_data[i].id == max_id)
            continue;
        if(share_data[i].average == max)
        {
            cout << "[Main thread] KeyDocID:" << share_data[i].id 
                 << " HighestJ:" << share_data[i].average << endl;
        }
    }

    duration = ((double)(clock() - start)) / CLOCKS_PER_SEC;
    duration *= 1000000;

    cout << "[Main thread] CPU time:" << duration << "ms" << endl;
	
	return 0;
}