#define SIZE 0xF0000

// I was too lazy to put some value inside the list[SIZE]

int main()
{
	double list[SIZE], sum = 0.;
	for (long i = 0; i < SIZE; i++) sum += list[i];

	/*
	double list[SIZE], sum1 = 0., sum2 = 0., sum3 = 0., sum4 = 0.;
	for (int i = 0; i < SIZE; i += 4) {
		sum1 += list[i];
		sum2 += list[i+1];
		sum3 += list[i+2];
		sum4 += list[i+3];
	}
	sum1 = (sum1 + sum2) + (sum3 + sum4);
	*/
	return 0;
}
