#include <iostream>
#include <vector>
#include <malloc.h>
#include <cmath>
#include <string>
using namespace std;

/**/
/**/
/**/
/**/
/**/


#define default_page_size 1024
#define default_pages_count 10


/**/
/**/
/**/
/**/
/**/
/**/
class Describer
{
	void *address;
	bool isEmpty;
public:
	void set_status(){ isEmpty = (isEmpty) ? false : true;}
	bool status(){ return isEmpty; }
	Describer(void *ptr);
};

Describer::Describer(void *ptr){
	isEmpty = true;
	address = ptr;
}




/**/
/**/
/**/
/**/
/**/
/**/

string info(bool isEmpty){
	if (isEmpty)
	{
		return "Empty";
	}
	else{
		return "Not empty";
	}
}

/**/
/**/
/**/
/**/
/**/
/**/
class Page
{
public:
	int b_size;
	int blocks;
	vector<bool> blocks_describer;
	void *location;
	Page(void* location);
	int get_b_size(){ return b_size; }
	int get_blocks(){ return blocks; }
	
};

Page::Page(void *loc){
	location = loc;
	b_size = default_page_size;
	blocks = 1;
	blocks_describer.push_back(true);
}





/**/
/**/
/**/
/**/
/**/
/**/
class log_node
{
public:
	int page;
	void *addr;
	int size;
	int position;
	log_node(int page,	void *addr,	int size, int position);
};

log_node::log_node(int page, void *addr, int size, int position){
	this->page = page;
	this->addr = addr;
	this->size = size;
	this->position = position;
}

/**/
/**/
/**/
/**/
/**/
/**/
class PageAllocator
{
	void *memory;
	vector<Describer> describers;
	vector<log_node> nodes;
	vector<Page> pages;
public:
	void mem_dump();
	void *mem_alloc(int size);
	void mem_free(void *addr);
	void *mem_realloc(void *addr, int size);
	int Type(int size);
	
	PageAllocator();

};


void PageAllocator::mem_dump(){
	cout << endl << "					MEMORY DUMP " << endl;

	for (int i = 0; i < pages.size(); ++i)
	{
		cout << "			PAGE " << i << endl;
		cout << "	Page status: " << info(describers[i].status()) << endl;
		cout << "	Blocks: " << pages[i].blocks << endl;
		cout << "	Block size: " << pages[i].b_size << endl;
		
		for (int j = 0; j < pages[i].blocks_describer.size(); ++j)
		{
			cout << "Block: " << j << endl;
			cout << "Status: " << info(pages[i].blocks_describer[j]) << endl;
			
		} 
	}
}


int get_blocks_size(int size){
	int b_size = 0;
	int i = 16;
	while (i <= ( default_page_size / 2 ))
	{

		if(size <= i){
			b_size = i;
			break;
		}
		else{
			i *= 2;
		}
	}

	return b_size;
}


void *PageAllocator::mem_alloc(int size){
	bool type = ( size > ( default_page_size / 2 ) ) ? false : true;
	//type one (true) - split on blocks
	//type two (false) - blocks on many pages
	if (type)
	{
		int b_size = get_blocks_size(size);

		int b_count = default_page_size / b_size;

		for(int i = 0; i < pages.size(); i++){
			if (pages[i].get_blocks() == b_count)
			{
				for (int j = 0; j < pages[i].blocks_describer.size(); ++j)
				{
					if (pages[i].blocks_describer[j])
					{
						pages[i].blocks_describer[j] = false;
						nodes.push_back(log_node(i,  (void*)(pages[i].location + pages[i].b_size * j), b_size, j));
						return (void*)(pages[i].location + pages[i].b_size * j);
					}
				}
			}
		}
		for (int i = 0; i < pages.size(); ++i)
		{
			if (describers[i].status())
			{
				pages[i].blocks = b_count;
				pages[i].b_size = b_size;
				vector<bool> tmp(b_count, true);
				pages[i].blocks_describer = tmp;
				pages[i].blocks_describer[0] = false;
				describers[i].set_status();
				nodes.push_back(log_node(i, pages[i].location, b_size, 0));
				return pages[i].location;
			}
		}
	}
	else{
		int pages_count = ceil((double)size / default_page_size);
		int tmp = 0;
		for (int i = 0; i < pages.size(); ++i)
		{
			if (describers[i].status())
			{
				tmp++;
				if (tmp == pages_count)
				{
					tmp--;	
					while(tmp >= 0){
						describers[i - tmp].set_status();
						pages[i - tmp].blocks_describer[0] = false;
						tmp--;
					}
					nodes.push_back(log_node(i - pages_count + 1, pages[i - pages_count + 1].location, pages_count * default_page_size, 0));
					return pages[i - pages_count + 1].location;
				}
			}
			else{ tmp = 0; }
		}
		cout << "Exception: No free memory." << endl;
		return nullptr;
	}
}


bool all_empty(vector<bool> v){
	for (int i = 0; i < v.size(); ++i)
	{
		if (!v[i]){	return false; }
	}
	return true;
}

void PageAllocator::mem_free(void *addr){
	for (int i = 0; i < nodes.size(); ++i)
	{
		if (addr == nodes[i].addr)
		{
			if (nodes[i].size > default_page_size / 2)
			{
				int pages_count = ceil((double)nodes[i].size / default_page_size);
				for (int j = 0; j < pages_count; ++j)
				{
					pages[nodes[i].page + j].blocks_describer[nodes[i].position] = true;
					describers[nodes[i].page + j].set_status();
				}
			}
			else{
				pages[nodes[i].page].blocks_describer[nodes[i].position] = true;
				if(all_empty(pages[nodes[i].page].blocks_describer)){
					pages[nodes[i].page].blocks_describer.clear();
					pages[nodes[i].page].blocks_describer.push_back(true);
					pages[nodes[i].page].b_size = default_page_size;
					pages[nodes[i].page].blocks = 1;
					describers[nodes[i].page].set_status();
				}
			}
			nodes.erase(nodes.begin() + i);
			break;
		}
	}
}


void *PageAllocator::mem_realloc(void *addr, int size){
	char *tmp_mem;
	for (int i = 0; i < nodes.size(); ++i)
	{
		if (addr == nodes[i].addr)
		{
			if (size == nodes[i].size)
			{
				return addr;
			}
			if (nodes[i].size > default_page_size / 2)
			{
				int pages_count = ceil((double)nodes[i].size / default_page_size);
				int pages_count2 = ceil((double)size / default_page_size);
				if (pages_count == pages_count2)
				{
					return addr;
				}
				if (pages_count2 > pages_count)
				{
					int tmp = 0;
					
					for (int j = nodes[i].page + pages_count; j < pages.size(); ++j)
					{
						if (describers[j + tmp].status())
						{
							tmp++;
							if (tmp == pages_count2 - pages_count)
							{
								nodes[i].size = pages_count2 * default_page_size;
								for (int k = 0; k < tmp; ++k)
								{
									pages[nodes[i].page + pages_count + k].blocks_describer[0] = false;
									describers[nodes[i].page + pages_count + k].set_status();
								}
								return addr;
							}
						}
						else{
							cout << "uefvsdfhb" << endl;
							break;
						}
					}
				}
				

				if (pages_count2 < pages_count)
				nodes[i].size = pages_count2 * default_page_size;
				{	
					for (int j = nodes[i].page + pages_count2; j <= (nodes[i].page + pages_count); ++j)
					{
						describers[j].set_status();
						pages[j].blocks_describer[0] = true;
						return addr;
					}
				}
			}
			tmp_mem = (char*)mem_alloc(size);
			if (tmp_mem == nullptr)
				{
				cout << "Reallocation exeption: Stack overflow." << endl;
				break;
			}
			else{
				for (int i = 0; i < size && i < nodes[i].size; ++i)
				{
					*(tmp_mem + i) = *(char*)(addr + i);
				}
				mem_free(addr);
				return (void*)tmp_mem;
			}
		}
	}
	return addr;
}

PageAllocator::PageAllocator(){
	memory = malloc(default_page_size * default_pages_count);
	
	for (int i = 0; i < default_pages_count; ++i)
	{
		pages.push_back(new Page((void*)(memory + default_page_size * i)));
		describers.push_back(new Describer(pages[i].location));
	}
}


int main(int argc, char const *argv[])
{

	PageAllocator pa;
	int* k = (int*)pa.mem_alloc(1000);
	pa.mem_dump();
	k = (int*)pa.mem_realloc((void*)k, 3000);
	pa.mem_dump();


	return 0;
}