//Halil Umut Özdemir,2016400168,Compiling,Working 
//The periodic boundaries and checkered-split are implemented
#include <cstdio>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <math.h>

#define S 360
#define INPUT_MESSAGE 0
#define END_MESSAGE 1
#define FIRST 2 + 8*turn
#define SECOND 3 + 8*turn
#define THIRD 4 + 8*turn
#define FOURTH 5 + 8*turn
#define FIFTH 6 + 8*turn
#define SIXTH 7 + 8*turn
#define SEVENTH 8 + 8*turn
#define EIGHTH 9 + 8*turn

#define MASTER_PROCESS 0

using namespace std;

void combine_matrices(int length, void* input_par, void* full_input_par,int* top, int* bottom, int* right, int* left, int top_right, int top_left, int bottom_right, int bottom_left){
    int (*input)[length] = static_cast<int (*)[length]>(input_par);
    int (*full_input)[length+2] = static_cast<int (*)[length+2]>(full_input_par);
    for(int i=0;i<length+2;i++){
        for(int j=0;j<length+2;j++){
            if(i==0&&j==0)
                full_input[i][j] = top_left;
            else if(i==0&&j==length+1)
                full_input[i][j] = top_right;
            else if(i==length+1&&j==0)
                full_input[i][j] = bottom_left;
            else if(i==length+1&&j==length+1)
                full_input[i][j] = bottom_right;
            else if(i==0)
                full_input[i][j] = top[j-1];
            else if(i==length+1)
                full_input[i][j] = bottom[j-1];
            else if(j==0)
                full_input[i][j] = left[i-1];
            else if(j==length+1)
                full_input[i][j] = right[i-1];
            else
                full_input[i][j] = input[i-1][j-1];
        }
    }
}

void compute_next_state(int length, void* input_par, void* full_input_par){
    int (*input)[length] = static_cast<int (*)[length]>(input_par);
    int (*full_input)[length+2] = static_cast<int (*)[length+2]>(full_input_par);    
    for(int i=1;i<=length;i++){
        for(int j=1;j<=length;j++){
            int neigh = full_input[i-1][j-1] + full_input[i-1][j] + full_input[i-1][j+1] + full_input[i][j-1] + full_input[i][j+1] 
                        + full_input[i+1][j-1] + full_input[i+1][j] + full_input[i+1][j+1];
                if(full_input[i][j]==1){
                    if(neigh<2){
                        input[i-1][j-1] = 0;
                    }else if(neigh>3){
                        input[i-1][j-1] = 0;
                    }
                }else{
                    if(neigh==3){
                        input[i-1][j-1] = 1;
                    }
                }
        }
    }
}

int main(int argc, char *argv[]){

    int ierr = MPI_Init(&argc,&argv);
    int procid,numprocs;
    ierr = MPI_Comm_rank(MPI_COMM_WORLD,&procid);
    ierr = MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    int number_of_turns = stoi(argv[3]);
    numprocs--;
    int sqrt_numprocs = (int)sqrt(numprocs);
    int length = S/sqrt_numprocs;
    if(procid==MASTER_PROCESS){
        ifstream file;
        file.open(argv[1]);
        int input[sqrt_numprocs][length][length];
        for(int i=0;i<sqrt_numprocs;i++){
            for(int j=0;j<length;j++)
                for(int k=0;k<S;k++)
                    file >> input[k/length][j][k%length];        
            for(int l=0;l<sqrt_numprocs;l++)
                MPI_Send(&input[l],length*length,MPI_INT,sqrt_numprocs*i+l+1,INPUT_MESSAGE,MPI_COMM_WORLD);
        }
        int output[S][S];
        int temp[length][length];
        //Receives all data from worker processes
        for(int i=0;i<numprocs;i++){
            ierr = MPI_Recv(&temp,length*length,MPI_INT,i+1,END_MESSAGE,MPI_COMM_WORLD,NULL);
            for(int j=0;j<length;j++)
                for(int k=0;k<length;k++)
                    output[(i/sqrt_numprocs)*length+j][(i%sqrt_numprocs)*length+k] = temp[j][k];
        }
        //Prints result to output file
        ofstream outfile;
        outfile.open(argv[2]);
        for(int i=0;i<S;i++){
            for(int j=0;j<S;j++)
                outfile << output[i][j] << " ";
            outfile << "\n";
        }
    }else{
        MPI_Status status;
        int input[length][length];
        ierr = MPI_Recv(&input,length*length,MPI_INT,MASTER_PROCESS,INPUT_MESSAGE,MPI_COMM_WORLD,&status);
        int top[length]; int bottom[length]; int right[length]; int left[length];
        int top_right; int bottom_right; int top_left; int bottom_left;
        for(int turn=0;turn < number_of_turns;turn++){
            //Right and Left Communication
            if(procid % 2){ //Odd processes
                int temp[length];
                //Copies its rightmost column
                for(int i=0;i<length;i++)
                    temp[i] = input[i][length-1];
                //Sends the rightmost column to the right process
                MPI_Send(&temp,length,MPI_INT,procid+1,FIRST,MPI_COMM_WORLD);
                //Receives right from the right process
                ierr = MPI_Recv(&right,length,MPI_INT,procid+1,FIRST,MPI_COMM_WORLD,&status);
                //Reveives the left part from the left process
                if(procid%sqrt_numprocs==1)
                    ierr = MPI_Recv(&left,length,MPI_INT,procid+sqrt_numprocs-1,SECOND,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&left,length,MPI_INT,procid-1,SECOND,MPI_COMM_WORLD,&status); 
                //Copies its leftmost column
                for(int i=0;i<length;i++)
                    temp[i] = input[i][0];
                //Sends the leftmost column to the left process
                if(procid%sqrt_numprocs==1)
                    MPI_Send(&temp,length,MPI_INT,procid+sqrt_numprocs-1,SECOND,MPI_COMM_WORLD);
                else
                    MPI_Send(&temp,length,MPI_INT,procid-1,SECOND,MPI_COMM_WORLD);

            }else{// Even Process
                //Reveives the left part from the left process
                ierr = MPI_Recv(&left,length,MPI_INT,procid-1,FIRST,MPI_COMM_WORLD,&status);
                int temp[length];
                //Copies its leftmost column
                for(int i=0;i<length;i++)
                    temp[i] = input[i][0];
                //Sends the leftmost column to the left process
                MPI_Send(&temp,length,MPI_INT,procid-1,FIRST,MPI_COMM_WORLD);
                //Copies its rightmost column
                for(int i=0;i<length;i++)
                    temp[i] = input[i][length-1];
                //Sends the rightmost column to the right process
                if(procid%sqrt_numprocs==0)
                    MPI_Send(&temp,length,MPI_INT,procid-sqrt_numprocs+1,SECOND,MPI_COMM_WORLD);
                else
                    MPI_Send(&temp,length,MPI_INT,procid+1,SECOND,MPI_COMM_WORLD);
                //Receives right from the right process
                if(procid%sqrt_numprocs==0)
                    ierr = MPI_Recv(&right,length,MPI_INT,procid-sqrt_numprocs+1,SECOND,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&right,length,MPI_INT,procid+1,SECOND,MPI_COMM_WORLD,&status);     
            }
            //Up and Down Communication
            int row_index = (procid-1)/sqrt_numprocs;
            if(row_index % 2){//Odd Rows(Indexes starts from zero)
                //Receivs its top from top process
                ierr = MPI_Recv(&top,length,MPI_INT,procid-sqrt_numprocs,THIRD,MPI_COMM_WORLD,&status);
                //Sends its down to down process
                if(row_index==sqrt_numprocs-1) //Last Row Process
                    MPI_Send(&input[length-1],length,MPI_INT,procid+sqrt_numprocs-numprocs,THIRD,MPI_COMM_WORLD);
                else
                    MPI_Send(&input[length-1],length,MPI_INT,procid+sqrt_numprocs,THIRD,MPI_COMM_WORLD);
                //Sends its up to up process
                MPI_Send(&input[0],length,MPI_INT,procid-sqrt_numprocs,FOURTH,MPI_COMM_WORLD);
                //Reveives its bottom from bottom process
                if(row_index==sqrt_numprocs-1) //Last Row Process
                    ierr = MPI_Recv(&bottom,length,MPI_INT,procid+sqrt_numprocs-numprocs,FOURTH,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&bottom,length,MPI_INT,procid+sqrt_numprocs,FOURTH,MPI_COMM_WORLD,&status);            
            }else{//Even Rows
                //Sends its down to the down process
                MPI_Send(&input[length-1],length,MPI_INT,procid+sqrt_numprocs,THIRD,MPI_COMM_WORLD);
                //Receives its top from top process
                if(row_index==0)
                    ierr = MPI_Recv(&top,length,MPI_INT,procid-sqrt_numprocs+numprocs,THIRD,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&top,length,MPI_INT,procid-sqrt_numprocs,THIRD,MPI_COMM_WORLD,&status);
                //Receives its bottom from top process
                ierr = MPI_Recv(&bottom,length,MPI_INT,procid+sqrt_numprocs,FOURTH,MPI_COMM_WORLD,&status);
                //Sends its up to up process
                if(row_index==0)
                    MPI_Send(&input[0],length,MPI_INT,procid-sqrt_numprocs+numprocs,FOURTH,MPI_COMM_WORLD);
                else
                    MPI_Send(&input[0],length,MPI_INT,procid-sqrt_numprocs,FOURTH,MPI_COMM_WORLD);  
            }
            //Cross Communication
            if(row_index % 2){//Odd Rows(Indexes starts from zero)
                //Receives data from top left
                if(procid%sqrt_numprocs==1)
                    ierr = MPI_Recv(&top_left,1,MPI_INT,procid-1,FIFTH,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&top_left,1,MPI_INT,procid-sqrt_numprocs-1,FIFTH,MPI_COMM_WORLD,&status);
                //Sends data to bottom right
                if(row_index==sqrt_numprocs-1){
                    if(procid%sqrt_numprocs==0)
                        MPI_Send(&input[length-1][length-1],1,MPI_INT,1,FIFTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[length-1][length-1],1,MPI_INT,procid+sqrt_numprocs+1-numprocs,FIFTH,MPI_COMM_WORLD);
                }else{
                    if(procid%sqrt_numprocs==0)
                        MPI_Send(&input[length-1][length-1],1,MPI_INT,procid+1,FIFTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[length-1][length-1],1,MPI_INT,procid+sqrt_numprocs+1,FIFTH,MPI_COMM_WORLD); 
                }
                //Receives data from top right
                if(procid%sqrt_numprocs==0)
                    ierr = MPI_Recv(&top_right,1,MPI_INT,procid-2*sqrt_numprocs+1,SIXTH,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&top_right,1,MPI_INT,procid-sqrt_numprocs+1,SIXTH,MPI_COMM_WORLD,&status);
                //Sends data to bottom left
                if(row_index==sqrt_numprocs-1){
                    if(procid%sqrt_numprocs==1)
                        MPI_Send(&input[length-1][0],1,MPI_INT,sqrt_numprocs,SIXTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[length-1][0],1,MPI_INT,procid-numprocs+sqrt_numprocs-1,SIXTH,MPI_COMM_WORLD);
                }else{
                    if(procid%sqrt_numprocs==1)
                        MPI_Send(&input[length-1][0],1,MPI_INT,procid+2*sqrt_numprocs-1,SIXTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[length-1][0],1,MPI_INT,procid+sqrt_numprocs-1,SIXTH,MPI_COMM_WORLD); 
                }
                //Send data to top right
                if(procid%sqrt_numprocs==0)
                    MPI_Send(&input[0][length-1],1,MPI_INT,procid-2*sqrt_numprocs+1,SEVENTH,MPI_COMM_WORLD);
                else
                    MPI_Send(&input[0][length-1],1,MPI_INT,procid-sqrt_numprocs+1,SEVENTH,MPI_COMM_WORLD);
                //Receives data from bottom left
                if(row_index==sqrt_numprocs-1){
                    if(procid%sqrt_numprocs==1)
                        MPI_Recv(&bottom_left,1,MPI_INT,sqrt_numprocs,SEVENTH,MPI_COMM_WORLD,&status);
                    else
                        MPI_Recv(&bottom_left,1,MPI_INT,procid-numprocs+sqrt_numprocs-1,SEVENTH,MPI_COMM_WORLD,&status);
                }else{
                    if(procid%sqrt_numprocs==1)
                        MPI_Recv(&bottom_left,1,MPI_INT,procid+2*sqrt_numprocs-1,SEVENTH,MPI_COMM_WORLD,&status);
                    else
                        MPI_Recv(&bottom_left,1,MPI_INT,procid+sqrt_numprocs-1,SEVENTH,MPI_COMM_WORLD,&status); 
                }
                //Receives data from bottom right
                if(row_index==sqrt_numprocs-1){
                    if(procid%sqrt_numprocs==0)
                        ierr = MPI_Recv(&bottom_right,1,MPI_INT,1,EIGHTH,MPI_COMM_WORLD,&status);
                    else
                        ierr = MPI_Recv(&bottom_right,1,MPI_INT,procid+sqrt_numprocs+1-numprocs,EIGHTH,MPI_COMM_WORLD,&status);
                }else{
                    if(procid%sqrt_numprocs==0)
                        ierr = MPI_Recv(&bottom_right,1,MPI_INT,procid+1,EIGHTH,MPI_COMM_WORLD,&status);
                    else
                        ierr = MPI_Recv(&bottom_right,1,MPI_INT,procid+sqrt_numprocs+1,EIGHTH,MPI_COMM_WORLD,&status); 
                }
                //Sends data to top left
                if(procid%sqrt_numprocs==1)
                    MPI_Send(&input[0][0],1,MPI_INT,procid-1,EIGHTH,MPI_COMM_WORLD);
                else
                    MPI_Send(&input[0][0],1,MPI_INT,procid-sqrt_numprocs-1,EIGHTH,MPI_COMM_WORLD);
            }else{//Even Rows
                //Sends data to bottom right
                if(procid%sqrt_numprocs==0)
                    MPI_Send(&input[length-1][length-1],1,MPI_INT,procid+1,FIFTH,MPI_COMM_WORLD);
                else
                    MPI_Send(&input[length-1][length-1],1,MPI_INT,procid+sqrt_numprocs+1,FIFTH,MPI_COMM_WORLD);
                //Receives data from top left
                if(row_index==0){
                    if(procid%sqrt_numprocs==1)
                        ierr = MPI_Recv(&top_left,1,MPI_INT,numprocs,FIFTH,MPI_COMM_WORLD,&status);
                    else
                        ierr = MPI_Recv(&top_left,1,MPI_INT,procid-sqrt_numprocs-1+numprocs,FIFTH,MPI_COMM_WORLD,&status);
                }else{
                    if(procid%sqrt_numprocs==1)
                        ierr = MPI_Recv(&top_left,1,MPI_INT,procid-1,FIFTH,MPI_COMM_WORLD,&status);
                    else
                        ierr = MPI_Recv(&top_left,1,MPI_INT,procid-sqrt_numprocs-1,FIFTH,MPI_COMM_WORLD,&status);
                }
                //Sends data to bottom left
                if(procid%sqrt_numprocs==1)
                    MPI_Send(&input[length-1][0],1,MPI_INT,procid-1+2*sqrt_numprocs,SIXTH,MPI_COMM_WORLD);
                else
                    MPI_Send(&input[length-1][0],1,MPI_INT,procid+sqrt_numprocs-1,SIXTH,MPI_COMM_WORLD);
                //Receives data from top right
                if(row_index==0){
                    if(procid%sqrt_numprocs==0)
                        ierr = MPI_Recv(&top_right,1,MPI_INT,numprocs-sqrt_numprocs+1,SIXTH,MPI_COMM_WORLD,&status);
                    else
                        ierr = MPI_Recv(&top_right,1,MPI_INT,procid-sqrt_numprocs+1+numprocs,SIXTH,MPI_COMM_WORLD,&status);
                }else{
                    if(procid%sqrt_numprocs==0)
                        ierr = MPI_Recv(&top_right,1,MPI_INT,procid-2*sqrt_numprocs+1,SIXTH,MPI_COMM_WORLD,&status);
                    else
                        ierr = MPI_Recv(&top_right,1,MPI_INT,procid-sqrt_numprocs+1,SIXTH,MPI_COMM_WORLD,&status);
                }
                //Receives data from bottom left
                if(procid%sqrt_numprocs==1)
                    ierr = MPI_Recv(&bottom_left,1,MPI_INT,procid-1+2*sqrt_numprocs,SEVENTH,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&bottom_left,1,MPI_INT,procid-1+sqrt_numprocs,SEVENTH,MPI_COMM_WORLD,&status);
                //Sends data to top right
                if(row_index==0){
                    if(procid%sqrt_numprocs==0)
                        MPI_Send(&input[0][length-1],1,MPI_INT,numprocs-sqrt_numprocs+1,SEVENTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[0][length-1],1,MPI_INT,procid-sqrt_numprocs+1+numprocs,SEVENTH,MPI_COMM_WORLD);
                }else{
                    if(procid%sqrt_numprocs==0)
                        MPI_Send(&input[0][length-1],1,MPI_INT,procid-2*sqrt_numprocs+1,SEVENTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[0][length-1],1,MPI_INT,procid-sqrt_numprocs+1,SEVENTH,MPI_COMM_WORLD);
                }
                //Sends data to top left
                if(row_index==0){
                    if(procid%sqrt_numprocs==1)
                        MPI_Send(&input[0][0],1,MPI_INT,numprocs,EIGHTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[0][0],1,MPI_INT,procid-sqrt_numprocs-1+numprocs,EIGHTH,MPI_COMM_WORLD);
                }else{
                    if(procid%sqrt_numprocs==1)
                        MPI_Send(&input[0][0],1,MPI_INT,procid-1,EIGHTH,MPI_COMM_WORLD);
                    else
                        MPI_Send(&input[0][0],1,MPI_INT,procid-sqrt_numprocs-1,EIGHTH,MPI_COMM_WORLD);
                } 
                //Receives data from bottom right
                if(procid%sqrt_numprocs==0)
                    ierr = MPI_Recv(&bottom_right,1,MPI_INT,procid+1,EIGHTH,MPI_COMM_WORLD,&status);
                else
                    ierr = MPI_Recv(&bottom_right,1,MPI_INT,procid+sqrt_numprocs+1,EIGHTH,MPI_COMM_WORLD,&status);
            }
            //Combines its partition and data comes from communication
            int full_input[length+2][length+2];
            combine_matrices(length,input,full_input,top,bottom,right,left,top_right,top_left,bottom_right,bottom_left);
            // Calculates the total value of neighbors of all entries and finds their next state
            compute_next_state(length,input,full_input);
        }
        //At the end of he all time steps sends data to master process
        MPI_Send(&input,length*length,MPI_INT,MASTER_PROCESS,END_MESSAGE,MPI_COMM_WORLD);
    }
    ierr = MPI_Finalize();
}