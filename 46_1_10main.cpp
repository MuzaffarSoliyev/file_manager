#include <iostream>
#include "os_file.h"

using namespace std;

int main()
{
	file_manager_t fm;
	setup_file_manager(&fm);

	char currentDir[256];

	

	
		fm.create(1000);
		fm.create_file("file1.txt", 1000);
		fm.create_file("file2.txt", 1); 
		fm.remove("file1.txt", 0);
		fm.remove("file1.txt", 0);

	system("pause");
	return 0;
}
