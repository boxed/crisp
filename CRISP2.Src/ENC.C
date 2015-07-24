#include <stdio.h>
#include <wtypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char	Copyright_Line[]={"This copy of CRISP is licensed to"};

char	Copyright_Usual[]={"\
Copyrightc Calidris\n\
Manhemsvägen 4\n\
SE-191 45 Sollentuna, Sweden\n\
Website: www.calidris-em.com\n\
Email: info@calidris-em.se\n\
Tel: +46 (8) 96 77 22\n\
Fax: +46 (8) 625 00 41\n\
Written by SoftHard Technology, 1992-2000\n\
Peter Oleynikov, 2001-2009\n\
Anders Hovmöller, 2010-2014\n"};

/*
char	Copyright_Usual[]={"\
Copyrightc Calidris\n\
Manhemsvagen 4\n\
SE-191 45 Sollentuna, Sweden\n\
Website: www.calidris-em.com\n\
Email: info@calidris-em.se\n\
Tel: +46 (8) 96 77 22\n\
Fax: +46 (8) 625 00 41\n\
Written by SoftHard Technology, 1992-2000\n\
and Peter Oleynikov, 2001-2009\n\n\
7 August 2009\n\n\
This copy of CRISP is licensed to"};

char	Copyright_FEI[]={"\
Copyrightc Calidris\n\
Manhemsvagen 4\n\
SE-191 45 Sollentuna, Sweden\n\
Website: www.calidris-em.com\n\
Email: info@calidris-em.se\n\
Tel: +46 (8) 96 77 22\n\
Fax: +46 (8) 625 00 41\n\
Written by SoftHard Technology, 1992-2000\n\
and Peter Oleynikov, 2001-2009\n\n\
7 August 2009 (with FEI files support)\n\n\
This copy of CRISP is licensed to"};

*/
char *Copyright = Copyright_Usual;

char DemoMsg[]="This is a demo version of CRISP2";
char DemoUser[]="CRISP2 Demo Version";
char Unlimited[]="Unlimited";
char Expired[]="Expired";
char Crippled[]="Crippled";
char Until[]="Valid until: %02d-%s-%d";

void main( int argc, char **argv )
{
	FILE *f;
	int len, i;
	BYTE b;
	int	XX=20;
	char dateStr[9];
	int month, day, year;
	char String[2048] = {0};
	char Date[256] = {0};
	const char *months[] = {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

	_strdate( dateStr);
	printf( "The current date is %s \n", dateStr);

	sscanf(dateStr, "%d/%d/%d", &month, &day, &year);
	sprintf(Date, "Version built: %d %s %d", day, months[month], 2000 + year);

	if( argc > 1 )
	{
		//Copyright = Copyright_FEI;
		printf("FEI Copyright notice\n");
	}
	strcpy(String, Copyright);
	strcat(String, Date);
	if( argc > 1 )
	{
		strcat(String, " (with FEI files support)");
	}
	strcat(String, "\n\n");
	strcat(String, Copyright_Line);
	Copyright = String;

	printf(Copyright);

	len=strlen(Copyright);
	f = fopen( "copyrt.c","w" );
	fprintf(f, "const BYTE Copyright[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = Copyright[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");

	len = strlen(DemoMsg);
	fprintf(f, "const BYTE szDemoMsg[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = DemoMsg[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");

	len = strlen(DemoUser);
	fprintf(f, "const BYTE szDemoUser[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = DemoUser[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");

	len = strlen(Until);
	fprintf(f, "const BYTE szUntil[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = Until[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");

	len = strlen(Unlimited);
	fprintf(f, "const BYTE szUnlimited[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = Unlimited[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");

	len = strlen(Expired);
	fprintf(f, "const BYTE szExpired[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = Expired[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");

	len = strlen(Crippled);
	fprintf(f, "const BYTE szCrippled[%d]={\n", len );
	for(i=0;i<len;i++) {
		b = Crippled[i] ^ (5+(i%7)+(i%23));
		fprintf(f, "%3d", b);
		if (i<len-1) {
			fprintf(f,",");
			if (i && (i%XX)==(XX-1)) fprintf(f,"\n");
		}
	}
	fprintf(f, "\n};\n");
	fclose( f );
}
