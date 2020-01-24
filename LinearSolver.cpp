#include <math.h>
#include <cstdio>
#include <iostream>
#include <fstream>

#define S 360

using namespace std;
void foo_workaround (int m, void *x)
{
    printf("Girdi\n");
    int (*arr)[m] = static_cast<int (*)[m]>(x);
    for(int i=0;i<m;i++)
        for(int j=0;j<m;j++)
            arr[i][j] = 2;
}
int main(int argc, char * argv[]){
    ifstream input_file;
    input_file.open(argv[1]);
    int number_of_turns = stoi(argv[3]);
    int input[S][S];
    for(int i=0; i<S;i++)
        for(int j=0;j<S;j++)
            input_file >> input[i][j];
    for(int turn=0;turn<number_of_turns;turn++){
        int temp[S][S];
        copy(&input[0][0], &input[0][0]+S*S,&temp[0][0]);
        printf("Turn %d:\nNeighbor Values:\n",turn+1);
        for(int i=0;i<S;i++){
            for(int j=0;j<S;j++){
                int neigh;
                if(i==0&&j==0){
                    neigh = input[S-1][S-1] + input[S-1][0] + input[S-1][1] + input[0][S-1] + input[1][S-1] + input[1][0] + input[1][1] + input[0][1];
                }else if(i==0&&j==S-1){
                    neigh = input[S-2][S-1] + input[S-1][S-1] + input[S-1][0] + input[0][0] + input[1][0] + input[1][S-1] + input[1][S-2] + input[0][S-2];
                }else if(i==S-1&&j==0){
                    neigh = input[0][0] + input[0][1] + input[S-1][1] + input[S-2][1] + input[S-2][0] + input[S-2][S-1] + input[S-1][S-1] + input[0][S-1];
                }else if(i==S-1&&j==S-1){
                    neigh = input[S-2][S-2] + input[S-2][S-1] + input[S-2][0] + input[S-1][0] + input[0][0] + input[0][S-1] + input[0][S-2] + input[S-1][S-2];
                }else if(i==0){
                    neigh = input[S-1][j-1] + input[S-1][j] + input[S-1][j+1] + input[0][j+1] + input[1][j+1] + input[1][j] + input[1][j-1] + input[0][j-1];
                }else if(i==S-1){
                    neigh = input[0][j-1] + input[0][j] + input[0][j+1] + input[S-1][j+1] + input[S-2][j+1] + input[S-2][j] + input[S-2][j-1] + input[S-1][j-1];
                }else if(j==0){
                    neigh = input[i+1][S-1] + input[i][S-1] + input[i-1][S-1] + input[i-1][0] + input[i-1][1] + input[i][1] + input[i+1][1] + input[i+1][0];
                }else if(j==S-1){
                    neigh = input[i+1][0] + input[i][0] + input[i-1][0] + input[i-1][S-1] + input[i-1][S-2] + input[i][S-2] + input[i+1][S-2] + input[i+1][S-1];
                }else{
                    neigh = input[i+1][j] + input[i+1][j-1] + input[i-1][j] + input[i-1][j-1] + input[i-1][j+1] + input[i+1][j+1] + input[i][j-1] + input[i][j+1];
                }
                if(temp[i][j]==1){
                    if(neigh<2){
                        input[i][j]=0;
                    }
                    else if(neigh>3){
                        input[i][j]=0;
                    }
                }else if(temp[i][j]==0){
                    if(neigh==3){
                        input[i][j]=1;
                    }
                }
                printf("%d ",neigh);
            }
            printf("\n");
        }
        printf("Map State:\n");
        for(int i=0;i<S;i++){
            for(int j=0;j<S;j++){
                printf("%d ",input[i][j]);
            }
            printf("\n");
        }
    }
    ofstream output_file;
    output_file.open(argv[2]);
     for(int i=0;i<S;i++){
        for(int j=0;j<S;j++){
            output_file << input[i][j] << " ";
        }
        output_file << "\n";
    }

    foo_workaround(6,input);
     for(int i=0;i<S;i++){
            for(int j=0;j<S;j++){
                printf("%d ",input[i][j]);
            }
            printf("\n");
        }

}