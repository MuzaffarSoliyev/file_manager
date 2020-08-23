#include <iostream>
#include <string>
#include <vector>
#include "os_file.h"
#pragma warning(disable:4996)

using namespace std;

struct MyTree
{
	string name;
	int size;
	MyTree *parent;
	vector <MyTree*> *childs;
	bool isDirectory;
	~MyTree()
	{
		if (isDirectory)
			delete childs;
	}
};

bool created=false;
MyTree *rootDir;
MyTree *currentDir;
int fsSize;
int Size_Busy();
void sizeRecursive(MyTree *node, int *size);
void recursiveDelete(MyTree *node);



int create(int disk_size)
{
	if (created) return 0;
	fsSize = disk_size;
	currentDir = new MyTree;
	created = true;
	currentDir->name = "";
	currentDir->parent = NULL;
	currentDir->isDirectory = true;
	currentDir->size = 0;
	currentDir->childs = new vector<MyTree*>();
	rootDir = currentDir;
	return 1;
}

int destroy()
{
	if (!created) return 0;
	
	recursiveDelete(rootDir);
	created = false;
	return 1;
}

MyTree* findDirFormPath(string path, string *lastDirName, int *foundDir)
{
	MyTree *thisDir;
	int i = 0;
	vector <string> dirs;
	string tmp;


	if (path[0] == '/')
	{
		thisDir = rootDir;
		i++;
	}
	else
	{
		thisDir = currentDir;
	}

	for (; i < path.length(); i++)
	{

		if (path[i] != '/')
		{
			tmp.push_back(path[i]);
		}
		else
		{
			dirs.push_back(tmp);
			tmp.clear();
		}
	}
	if (!tmp.empty())
		dirs.push_back(tmp);


	tmp.clear();

	int k = 0;
	bool breaked = false;
	for (; k < dirs.size(); k++)
	{
		bool changed = false;
		if (dirs[k].compare("..") == 0)
		{
			changed = true;
			if (thisDir->parent != nullptr)
				thisDir = thisDir->parent;
			continue;
		}
		if (dirs[k].compare(".") == 0)
		{
			changed = true;
			continue;
		}

		if (thisDir->isDirectory)
		{
			for (auto j = thisDir->childs->begin(); j != thisDir->childs->end(); j++)
			{
				if ((*j)->name == dirs[k])
				{
					changed = true;
					thisDir = (*j);
					break;
				}
			}
		}

		if (!changed)
		{
			breaked = true;
			break;
		}
		
	}

	if (!breaked && k > 0)
		--k;
	if (dirs.size() > 0)
		*lastDirName = dirs[k];
	else
		*lastDirName = "";

	if (breaked)
	{
		if (k == (dirs.size() - 1))
		{
			*foundDir = 1;
			return thisDir;
		}
		*foundDir = 2;
		return thisDir;
	}
	*foundDir = 0;
	return thisDir;
}

int create_dir(const char* dir_name)
{
	if (!created) return 0;
	int foundDir;
	string lastDirName;
	MyTree *foundNode = findDirFormPath(dir_name, &lastDirName, &foundDir);
	if (foundDir == 0 || foundDir == 2 || !foundNode->isDirectory) return 0;

	MyTree *NewDir = new MyTree;
	NewDir->name = lastDirName;
	NewDir->isDirectory = true;
	NewDir->parent = foundNode;
	NewDir->childs = new vector<MyTree*>();
	foundNode->childs->push_back(NewDir);
	return 1;
}

int create_file(const char* file_name, int file_size)
{
	if (!created || file_size==0 || file_size > fsSize - Size_Busy()) return 0;
	int foundDir;
	string lastDirName;
	MyTree *foundNode = findDirFormPath(file_name, &lastDirName, &foundDir);
	if (foundDir == 0 || foundDir == 2 || !foundNode->isDirectory) return 0;

	MyTree *NewFile = new MyTree;
	NewFile->name = lastDirName;
	NewFile->isDirectory = false;
	NewFile->parent = foundNode;
	NewFile->size = file_size;
	foundNode->childs->push_back(NewFile);
	return 1;
}

int change_dir(const char* path)
{
	int foundDir;
	string lastDirName;
	if (!created) return 0;
	MyTree *foundNode = findDirFormPath(path, &lastDirName, &foundDir);
	if (foundDir != 0 || !foundNode->isDirectory) return 0;
	currentDir = foundNode;
	return 1;
}

void get_cur_dir(char *dst)
{
	string fullPath = "";
	rootDir = currentDir;
	while (rootDir->parent != NULL)
	{
		fullPath.insert(0, rootDir->name + "/");
		rootDir = rootDir->parent;
	}
	fullPath.insert(0, "/");
	strncpy(dst, fullPath.c_str(), fullPath.length() + 1);
	fullPath.clear();
}

int remove(const char* name, int recursive)
{
	int foundDir;
	string lastDirName;
	if (!created) return 0;
	MyTree *foundNode = findDirFormPath(name, &lastDirName, &foundDir);
	if (foundDir > 0) return 0;
	if (foundNode->isDirectory && recursive == 0) return -1;
	if (!foundNode->isDirectory)
	{
		currentDir = foundNode->parent;
		fsSize = Size_Busy() - foundNode->size;
		delete foundNode;
		return 1;
	}
	else
	{
		recursiveDelete(foundNode);
	}
	return 0;
	
}

void recursiveDelete(MyTree *node)
{
	for (auto i = node->childs->begin(); i != node->childs->end(); i++)
	{
		if (!(*i)->isDirectory) 
		{
			fsSize = Size_Busy() - (*i)->size;
			delete (*i);
		}
		else
		{
			recursiveDelete(*i);
		}
	}
	delete node;
}

int size(const char *path)
{
	int foundDir;
	string lastDirName;
	if (!created) return -1;
	MyTree *foundNode= findDirFormPath(path, &lastDirName, &foundDir);
	if (foundDir != 0) return -1;
	if (!foundNode->isDirectory) return foundNode->size;
	else
	{
		int size = 0;
		sizeRecursive(foundNode, &size);
		return size;
	}

}

void sizeRecursive(MyTree *node, int *size)
{
	for (auto j = node->childs->begin(); j != node->childs->end(); j++)
	{
		if (!(*j)->isDirectory)
		{
			(*size) += (*j)->size;
		}
		else
		{
			sizeRecursive((*j), size);
		}
	}
}

int Size_Busy()
{
	if (!created) return -1;
	int size = 0;
	sizeRecursive(rootDir, &size);
	return size;
}

void setup_file_manager(file_manager_t *fm)
{
	fm->create = create;
	fm->destroy = destroy;
	fm->create_dir = create_dir;
	fm->create_file = create_file;
	fm->remove = remove;
	fm->change_dir = change_dir;
	fm->get_cur_dir = get_cur_dir;
	fm->size = size;
}