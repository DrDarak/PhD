#include <iostream.h>

template <class T>
void sift(T *a, int i, int n)
{	int j;
	T x = a[i];
	while((j=2*i+1)<n)
	{	if(j<n-1 && a[j] < a[j+1]) j++;
		if(a[j] < x) break;
		a[i] = a[j];
		i = j;
	}
	a[i] = x;
}

template <class T>
void show(T *a, int n)
{	for(int i=0; i<n; i++) cout << a[i] << " ";
	cout << endl;
}

template <class T>
void heapsort(T *a, int n)
{	int nStore = n;
	for(int i=n/2-1; i>=0; i--) // generate heap
	{	sift(a,i,n);
		show(a,n);
	}
	while(--n)	// sort
	{	T x = a[0];

		a[0] = a[n];
		a[n] = x;
		sift(a,0,n);
		show(a,nStore);
	}
}

int main()
{	int a[8];
	cout << "enter 8 ints: ";
	for(int i=0; i<8; i++) cin >> a[i];
	heapsort(a,8);
}
