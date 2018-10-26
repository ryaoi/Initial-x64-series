#include <time.h>
#include <stdio.h>

void mul1(void);
void mul2(void);
void mul3(void);
void mul4(void);
void mul5(void);
void mul6(void);

int main()
{
	printf("====MULTIPLICATION BY 2=========\n");
	printf("[MUL 1] Use 'imul' instruction\n");
	clock_t begin = clock();
	for (long i = 0; i < 1000000000; i++)
		mul1();
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[MUL 1] Time : %f\n", time_spent);

	printf("[MUL 2] Use 'mov, add' instruction\n");
	begin = clock();
	for (long i = 0; i < 1000000000; i++)
		mul2();
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[MUL 2] Time : %f\n", time_spent);

	printf("[MUL 3] Use 'shl' instruction\n");
	begin = clock();
	for (long i = 0; i < 1000000000; i++)
		mul3();
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[MUL 3] Time : %f\n", time_spent);

	printf("[MUL 4] Use 'lea' instruction\n");
	begin = clock();
	for (long i = 0; i < 1000000000; i++)
		mul4();
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[MUL 4] Time : %f\n", time_spent);


	printf("====MULTIPLICATION BY 5=========\n");
	printf("[MUL 5] Use 'imul' instruction\n");
	begin = clock();
	for (long i = 0; i < 1000000000; i++)
		mul1();
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[MUL 5] Time : %f\n", time_spent);

	printf("[MUL 6] Use 'lea' instruction\n");
	begin = clock();
	for (long i = 0; i < 1000000000; i++)
		mul2();
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[MUL 6] Time : %f\n", time_spent);


	return (0);
}
