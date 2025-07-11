#include <stdio.h>
int main(){

int a;
printf("Please enter a number");
scanf("%d", &a);
printf("%d\n", a);


int sum = 0;
for(int i=1; i<a; i++){
	int mod = a % i;
	if(mod==0){
	  sum+=i;
	}
}

if(sum==a){
  printf("%d is a perfect number", sum);
}else{
 printf("%d is not a perfect number", sum);
}
 


return 0;
}
