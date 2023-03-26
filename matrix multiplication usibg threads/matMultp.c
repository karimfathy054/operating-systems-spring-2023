#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <pthread.h>

FILE *matA, *matB, *matC_by_matrix, *matC_by_row, *matC_by_element;
int matrixA[20][20], matrixB[20][20], matrixC1[20][20], matrixC2[20][20], matrixC3[20][20];
int cRows, cCols, cIndex;
typedef struct
{
    int rows, cols;
} dims;

dims getMatrix(FILE *matrixFile, int matrix[][20])
{
    int rows, columns;
    fscanf(matrixFile, "row=%d col=%d", &rows, &columns);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            fscanf(matrixFile, "%d", &matrix[i][j]);
        }
    }
    dims matDimms;
    matDimms.rows = rows;
    matDimms.cols = columns;
    return matDimms;
}

void *resultByMatrix(void *arg)
{
    for (int i = 0; i < cRows; i++)
    {
        for (int j = 0; j < cCols; j++)
        {
            matrixC1[i][j] = 0;
            for (int k = 0; k < cIndex; k++)
            {
                matrixC1[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    return NULL;
}
void *resultByRow(void *arg)
{
    dims *dimensions = (dims *)arg;
    int rowNumber =dimensions->rows;
    for (int j = 0; j < cRows; j++)
    {
        matrixC2[rowNumber][j] = 0;
        for (int k = 0; k < cIndex; k++)
        {
            matrixC2[rowNumber][j] += matrixA[rowNumber][k] * matrixB[k][j];
        }
    }
    return NULL;
}
void *resultByElement(void *arg)
{
    dims* dimensions = (dims*)arg;
    int rowNumber = dimensions->rows;
    int columnNumber = dimensions->cols;

    matrixC3[rowNumber][columnNumber] = 0;
    for (int k = 0; k < cIndex; k++)
    {
        matrixC3[rowNumber][columnNumber] += matrixA[rowNumber][k] * matrixB[k][columnNumber];
    }
    return NULL;
}

void writeToFile(FILE *file, int matrix[][20]);

void oneThreadOneMatrix()
{
    printf("thread per matrix:\n");
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    pthread_t oneThread;
    pthread_create(&oneThread, NULL, &resultByMatrix, NULL);
    pthread_join(oneThread, NULL);

    gettimeofday(&stop, NULL);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
     
    writeToFile(matC_by_matrix, matrixC1);
}

void RThreadOneMatrix()
{
    printf("\n");
    printf("thread per row:\n");
    dims temp[cRows];
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    pthread_t oneRow[cRows];
    for (int i = 0; i < cRows; i++)
    {
        temp[i].rows=i;
        pthread_create(&oneRow[i], NULL, &resultByRow, &temp[i]);
    }

    for (int i = 0; i < cRows; i++)
    {
        pthread_join(oneRow[i], NULL);
    }

    gettimeofday(&stop, NULL);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    
    writeToFile(matC_by_row, matrixC2);
}

void elementForElement()
{
    printf("\n");
    printf("thread per element:\n");
    dims temp[cRows][cCols];
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    pthread_t totalThreads[cRows][cCols];

    for (int i = 0; i < cRows; i++)
    {
        for (int j = 0; j < cCols; j++)
        {
            
            temp[i][j].rows=i;
            temp[i][j].cols=j;
            pthread_create(&totalThreads[i][j], NULL, &resultByElement, &temp[i][j]);
        }
    }

    for (int i = 0; i < cRows; i++)
    {
        for (int j = 0; j < cCols; j++)
        {
            pthread_join(totalThreads[i][j], NULL);
        }
    }

    gettimeofday(&stop, NULL);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    
    writeToFile(matC_by_element, matrixC3);
}

void writeToFile(FILE *file, int matrix[][20])
{

    fprintf(file, "rows=%d cols=%d\n", cRows, cCols);
    for (int i = 0; i < cRows; i++)
    {
        for (int j = 0; j < cCols; j++)
        {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fflush(file);
    fclose(file);
}

void start(int argc, char *argv[])
{

    if (argc <= 1)
    {
        matA = fopen("a.txt", "r");
        matB = fopen("b.txt", "r");
        matC_by_matrix = fopen("c_per_matrix.txt", "w");
        matC_by_row = fopen("c_per_row.txt", "w");
        matC_by_element = fopen("c_per_element.txt", "w");
        if (matA == NULL || matB == NULL || matC_by_element == NULL || matC_by_matrix == NULL || matC_by_row == NULL)
        {
            printf("file problem\n");
            exit(1);
        }
    }
    else if (argc == 4)
    {
        char *dir = (char *)malloc(1000);
        strcpy(dir, argv[1]);
        matA = fopen(strcat(dir, ".txt"), "r");
        strcpy(dir, argv[2]);
        matB = fopen(strcat(dir, ".txt"), "r");
        strcpy(dir, argv[3]);
        matC_by_matrix = fopen(strcat(dir, "_per_matrix.txt"), "w");
        strcpy(dir, argv[3]);
        matC_by_row = fopen(strcat(dir, "_per_row.txt"), "w");
        strcpy(dir, argv[3]);
        matC_by_element = fopen(strcat(dir, "_per_element.txt"), "w");
        free(dir);
    }
    else{
        printf("invalid arguments\nPROCEEDING WITH DEFAULT NAMES\n");
        matA = fopen("a.txt", "r");
        matB = fopen("b.txt", "r");
        matC_by_matrix = fopen("c_per_matrix.txt", "w");
        matC_by_row = fopen("c_per_row.txt", "w");
        matC_by_element = fopen("c_per_element.txt", "w");
        if (matA == NULL || matB == NULL || matC_by_element == NULL || matC_by_matrix == NULL || matC_by_row == NULL)
        {
            printf("file problem\n");
            exit(1);
        }
    }
    

    fprintf(matC_by_matrix, "Method: A thread per matrix\n");
    fprintf(matC_by_row, "Method: A thread per row\n");
    fprintf(matC_by_element, "Method: A thread per element\n");
    dims a_dimensions = getMatrix(matA, matrixA);
    dims b_dimensions = getMatrix(matB, matrixB);
    if (a_dimensions.cols != b_dimensions.rows)
    {
        printf("not possible to multiply\n");
        exit(1);
    }
    cRows = a_dimensions.rows;
    cCols = b_dimensions.cols;
    cIndex = a_dimensions.cols;
}

int main(int argc, char *argv[])
{

    start(argc, argv);
    oneThreadOneMatrix();
    RThreadOneMatrix();
    elementForElement();

    return 0;
}
