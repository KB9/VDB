int branchlessReturn(int a, int b)
{
	return a + b;
}

int branchedReturn(int a, int b)
{
	if (a < b)
		return b;
	else
		return a;
}

void recursive(int i)
{
	if (i > 0)
		return recursive(i-1);
	else
		return;
}

int main(int argc, char* argv[])
{
	branchlessReturn(2, 3);
	branchedReturn(4, 5);
	recursive(3);
}