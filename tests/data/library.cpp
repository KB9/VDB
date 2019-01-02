int library_value = 0;

int getLibraryValue()
{
	return library_value;
}

void __attribute__((constructor)) initLibrary()
{
	library_value = 10;
}

void __attribute__((destructor)) cleanUpLibrary()
{
	library_value = 20;
}