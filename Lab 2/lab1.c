#include <stdio.h>
int main(){
	printf("Hello World\n");
	
	int a  =10;
	int b = 5;
	int total = a+b;
	int i = 0;
	
	printf("The total is %d\n", total);
	
	printf("Please Enter a Value :");
	scanf("%d", &total);
	
	//printf("The total is %d\n", total, sizeof(ab));
	
	
	printf("The total is %d and Size of it is %d\n", total, sizeof(total));
	
	int c = 0;
	int d = 0;
	printf("Please Enter a Value :");
	scanf("%d", &c);
	printf("Please Enter a Value :");
	scanf("%d", &d);
	
	if(c>d){
	printf("%d is Bigger than %d\n", c, d);
	}
	else if(d>c){
	printf("%d is Bigger than %d\n", d, c);
	}else{
	printf("%d is equal to %d\n", c, d);
	}
	
	
	while(i<5){
	printf("Iteration %d\n", i);
	i++;
	}
	
	for(int j=0; j < 5; j++){
	printf("Increment %d\n", j);
	}
	
//	array
	
	int arr[] = {10, 20, 30};
	
	for(int k = 0; k<3; k++){
	printf("%d\n", arr[k]);
	}
	
	int arr1[3];
	
	for(int l = 0; l<3; l++){
	printf("index %d input - ", l);
	scanf("%d", &arr1[l]);
	}
	
	for(int k = 0; k<3; k++){
	printf("%d\n", arr1[k]);
	}
	
	printf("%d\n", sizeof(arr1));
	int arr2[] = {10, 20, 30};
	int length = sizeof(arr2) / sizeof(arr2[0]);
	printf("%d\n", length);
	
	return 0;
}
