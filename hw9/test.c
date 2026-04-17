#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int global_one=1;
long global_twos=0x0202020202020202;
char username[40];

const char *roasts[] = {
    "You move like a sloth that's never in a hurry.",
    "You dance like a robot with loose batteries.",
    "You cook like a mad scientist on a budget.",
    "You dress like a colorful thrift store explosion.",
    "You sing like a cat discovering opera.",
    "You have the coordination of a puppy on ice.",
    "You tell jokes with peak dad energy.",
    "You show up fashionably late every time.",
    "You have the memory of a very optimistic goldfish.",
    "You navigate with the confidence of a lost tourist."
};

int main(int argc, char** argv) {
	printf("Hello, what is your first name please? ");
	fflush(stdout);
	fgets(username,sizeof(username),stdin);
	username[strcspn(username, "\n")] = '\0';
	printf("Hi there %s, pleased to meet you.\n",username);
	char filename[50];
	sprintf(filename,"%s.txt",username);

	FILE *surprise = fopen(filename,"w");
	srand(time(NULL));	

	fprintf(surprise,"%s\n",roasts[rand()%10]);
	fclose(surprise);

	sleep(10);

}
