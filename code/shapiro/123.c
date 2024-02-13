#include <stdio.h>
#include <math.h>

#define uc		unsigned char
#define limit(x)	(x > 255.0 ? 255.0 : (x < 0.0 ? 0.0 : rint(x)))

uc	**Y;
uc	**U;
uc	**V;
uc	**RGB;

uc	**newuc(r,c)
int	r;
int	c;
{
	uc	**new;
	int	i;

	new = (uc **)(malloc(sizeof(uc *) * r));

	for(i = 0 ; i < r ; i++)
		new[i] = (uc *)(malloc(sizeof(uc) * c));

	return new;
}

uc	**iread(fp, x, y)
FILE	*fp;
int	*x;
int	*y;
{
	uc	**new;
	char	dummy[256];
	int	i;
	int	j;

	for(i = 0 ; i < 3 ; i++)
	{
		for(dummy[0] = '#' ; dummy[0] == '#' ;)
		{
			fgets(dummy, 255, fp);
			printf("header:\t%s", dummy);
		}

		if(i == 1)
			sscanf(dummy, "%d %d", y, x);
	}

	new = newuc(*x, *y * 3);

	for(i = 0 ; i < *x ; i++)
	for(j = 0 ; j < *y * 3 ; j++)
		new[i][j] = fgetc(fp);

	return new;
}

int main(argc, argv)
int	argc;
char	**argv;
{
	FILE	*yp;
	FILE	*up;
	FILE	*vp;
	FILE	*rgbp;

	float	yf;
	float	uf;
	float	vf;
	float	R;
	float	G;
	float	B;

	int	yy;
	int	xx;

	int	i;
	int	j;

	int	sub;

	printf("usage: %s <rgb> <y> <u> <v>\n", argv[0]);

	rgbp = fopen(argv[1], "r");
	yp = fopen(argv[2], "w");
	up = fopen(argv[3], "w");
	vp = fopen(argv[4], "w");

	RGB = iread(rgbp, &xx, &yy);

	Y = newuc(xx, yy);
	U = newuc(xx, yy);
	V = newuc(xx, yy);

	printf("rgb: %d %d\n", xx, yy);

	for(i = 0 ; i < xx ; i++)
	for(j = 0 ; j < yy ; j++)
	{
		R = (float)RGB[i][j * 3 + 0];
		G = (float)RGB[i][j * 3 + 1];
		B = (float)RGB[i][j * 3 + 2];

		yf =  0.2990 * R + 0.5870 * G + 0.1140 * B;
 		uf = -0.1686 * R - 0.3311 * G + 0.4997 * B;
		vf =  0.4998 * R - 0.4185 * G - 0.0813 * B;

		Y[i][j] = (uc)limit(yf);
		U[i][j] = (uc)limit(uf + 128.0);
		V[i][j] = (uc)limit(vf + 128.0);
	}

	fprintf(yp, "P5\n# y file created by 123\n%d %d\n255\n", yy, xx);
	fprintf(up, "P5\n# u file created by 123\n%d %d\n255\n", yy, xx);
	fprintf(vp, "P5\n# v file created by 123\n%d %d\n255\n", yy, xx);

	for(i = 0 ; i < xx ; i++)
	for(j = 0 ; j < yy ; j++)
	{
		fputc(Y[i][j], yp);
		fputc(U[i][j], up);
		fputc(V[i][j], vp);
	}

	fclose(yp);
	fclose(up);
	fclose(vp);
}
