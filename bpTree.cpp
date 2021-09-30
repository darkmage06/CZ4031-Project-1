#include <iostream>
#include <string>
#include <climits>
#include <vector>
#include "bufferPool.h"

using namespace std;
const int MAX = 5; //size of each node

class Node
{
	bool IS_LEAF;
	int* key, size;
	Address* address;
	Node** ptr;
	friend class BPTree;

public:
	Node() {
		//dynamic memory allocation
		key = new int[MAX];
		address = new Address[MAX];
		ptr = new Node*[MAX + 1];
	}
};

class BPTree
{
	Node* root;
	int numOfNode = 0, treeLvl = 0, numOfNodeDel = 0;

public:
	BPTree()
	{
		root = NULL;
	}

	~BPTree()
	{
		//calling cleanUp routine
		cleanUp(root);
	}

	void insertInternal(int x, Address address, Node* cursor, Node* child)
	{
		if (cursor->size < MAX)
		{
			//if cursor is not full
			//find the correct position for new key
			int i = 0;
			while (x > cursor->key[i] && i < cursor->size) i++;
			//make space for new key
			for (int j = cursor->size;j > i; j--)
			{
				cursor->key[j] = cursor->key[j - 1];
				cursor->address[j] = cursor->address[j - 1];
			}
			//make space for new pointer
			for (int j = cursor->size + 1; j > i + 1; j--)
			{
				cursor->ptr[j] = cursor->ptr[j - 1];
			}
			cursor->key[i] = x;
			cursor->address[i] = address;
			cursor->size++;
			cursor->ptr[i + 1] = child;
		}
		else
		{
			//if overflow in internal node
			//create new internal node
			Node* newInternal = new Node;
			//create virtual Internal Node;
			int virtualKey[MAX + 1];
			Address tempAddressList[MAX + 1];
			Node* virtualPtr[MAX + 2];
			//increase number of node by 1
			numOfNode += 1;
			for (int i = 0; i < MAX; i++)
			{
				virtualKey[i] = cursor->key[i];
				tempAddressList[i] = cursor->address[i];
			}
			for (int i = 0; i < MAX + 1; i++)
			{
				virtualPtr[i] = cursor->ptr[i];
			}
			int i = 0, j;
			while (x > virtualKey[i] && i < MAX) i++;
			//make space for new key
			for (int j = MAX + 1;j > i; j--)
			{
				virtualKey[j] = virtualKey[j - 1];
				tempAddressList[j] = tempAddressList[j - 1];
			}
			virtualKey[i] = x;
			tempAddressList[i] = address;
			//make space for new ptr
			for (int j = MAX + 2;j > i + 1; j--)
			{
				virtualPtr[j] = virtualPtr[j - 1];
			}
			virtualPtr[i + 1] = child;
			newInternal->IS_LEAF = false;
			//split cursor into two nodes
			cursor->size = (MAX + 1) / 2;
			newInternal->size = MAX - ((MAX + 1) / 2);
			//give elements and pointers to the new node
			for (i = 0, j = cursor->size + 1; i < newInternal->size; i++, j++)
			{
				newInternal->key[i] = virtualKey[j];
				newInternal->address[i] = tempAddressList[j];
			}
			for (i = 0, j = cursor->size + 1; i < newInternal->size + 1; i++, j++)
			{
				newInternal->ptr[i] = virtualPtr[j];
			}
			// m = cursor->key[cursor->size]
			if (cursor == root)
			{
				//if cursor is a root node, we create a new root
				Node* newRoot = new Node;
				newRoot->key[0] = child->key[0];
				newRoot->address[0] = child->address[0];
				newRoot->ptr[0] = cursor;
				newRoot->ptr[1] = newInternal;
				newRoot->IS_LEAF = false;
				newRoot->size = 1;
				root = newRoot;
				//increase number of node and tree level by 1
				numOfNode += 1;
				treeLvl += 1;
			}
			else
			{
				//recursion
				//find depth first search to find parent of cursor
				insertInternal(cursor->key[cursor->size], cursor->address[cursor->size], findParent(root, cursor), newInternal);
			}
		}
	}

	void removeInternal(int x, Address address, Node* cursor, Node* child, bufferPool *bufferPool)
	{
		//deleting the key x first
		//checking if key from root is to be deleted
		if (cursor == root)
		{
			if (cursor->size == 1)//if only one key is left, change root
			{
				treeLvl -= 1;
				numOfNode -= 1;
				numOfNodeDel += 1;
				if (cursor->ptr[1] == child)
				{
					root = cursor->ptr[0];
					return;
				}
				else if (cursor->ptr[0] == child)
				{
					root = cursor->ptr[1];
					return;
				}
			}
		}
		int pos;
		for (pos = 0; pos < cursor->size; pos++)
		{
			if (cursor->key[pos] == x)
			{
				break;
			}
		}
		for (int i = pos; i < cursor->size; i++)
		{
			cursor->key[i] = cursor->key[i + 1];
			cursor->address[i] = cursor->address[i + 1];
		}
		//now deleting the pointer child
		for (pos = 0; pos < cursor->size + 1; pos++)
		{
			if (cursor->ptr[pos] == child)
			{
				break;
			}
		}
		for (int i = pos; i < cursor->size + 1; i++)
		{
			cursor->ptr[i] = cursor->ptr[i + 1];
		}
		cursor->size--;
		if (cursor->size >= (MAX + 1) / 2 - 1)//no underflow
		{
			return;
		}
		//underflow, try to transfer first
		if (cursor == root)return;
		Node* parent = findParent(root, cursor);
		int leftSibling, rightSibling;
		//finding left n right sibling of cursor
		for (pos = 0; pos < parent->size + 1; pos++)
		{
			if (parent->ptr[pos] == cursor)
			{
				leftSibling = pos - 1;
				rightSibling = pos + 1;
				break;
			}
		}
		//try to transfer
		if (leftSibling >= 0)//if left sibling exists
		{
			Node* leftNode = parent->ptr[leftSibling];
			//check if it is possible to transfer
			if (leftNode->size >= (MAX + 1) / 2)
			{
				//make space for transfer of key
				for (int i = cursor->size; i > 0; i--)
				{
					cursor->key[i] = cursor->key[i - 1];
					cursor->address[i] = cursor->address[i - 1];
				}
				//transfer key from left sibling through parent
				cursor->key[0] = parent->key[leftSibling];
				cursor->address[0] = parent->address[leftSibling];
				parent->key[leftSibling] = leftNode->key[leftNode->size - 1];
				parent->address[leftSibling] = leftNode->address[leftNode->size - 1];
				//transfer last pointer from leftnode to cursor
				//make space for transfer of ptr
				for (int i = cursor->size + 1; i > 0; i--)
				{
					cursor->ptr[i] = cursor->ptr[i - 1];
				}
				//transfer ptr
				cursor->ptr[0] = leftNode->ptr[leftNode->size];
				cursor->size++;
				leftNode->size--;
				return;
			}
		}
		if (rightSibling <= parent->size)//check if right sibling exist
		{
			Node* rightNode = parent->ptr[rightSibling];
			//check if it is possible to transfer
			if (rightNode->size >= (MAX + 1) / 2)
			{
				//transfer key from right sibling through parent
				cursor->key[cursor->size] = parent->key[pos];
				cursor->address[cursor->size] = parent->address[pos];
				parent->key[pos] = rightNode->key[0];
				parent->address[pos] = rightNode->address[0];
				for (int i = 0; i < rightNode->size - 1; i++)
				{
					rightNode->key[i] = rightNode->key[i + 1];
					rightNode->address[i] = rightNode->address[i + 1];
				}
				//transfer first pointer from rightnode to cursor
				//transfer ptr
				cursor->ptr[cursor->size + 1] = rightNode->ptr[0];
				for (int i = 0; i < rightNode->size; ++i)
				{
					rightNode->ptr[i] = rightNode->ptr[i + 1];
				}
				cursor->size++;
				rightNode->size--;
				return;
			}
		}
		//transfer wasnt posssible hence do merging
		if (leftSibling >= 0)
		{
			//leftnode + parent key + cursor
			Node* leftNode = parent->ptr[leftSibling];
			leftNode->key[leftNode->size] = parent->key[leftSibling];
			leftNode->address[leftNode->size] = parent->address[leftSibling];
			for (int i = leftNode->size + 1, j = 0; j < cursor->size; j++)
			{
				leftNode->key[i] = cursor->key[j];
				leftNode->address[i] = cursor->address[j];
			}
			for (int i = leftNode->size + 1, j = 0; j < cursor->size; j++)
			{
				leftNode->ptr[i] = cursor->ptr[j];
				cursor->ptr[j] = NULL;
			}
			leftNode->size += cursor->size + 1;
			cursor->size = 0;
			//delete cursor
			removeInternal(parent->key[leftSibling], parent->address[leftSibling], parent, cursor, bufferPool);
			numOfNode -= 1;
			numOfNodeDel += 1;
		}
		else if (rightSibling <= parent->size)
		{
			//cursor + parent key + rightnode
			Node* rightNode = parent->ptr[rightSibling];
			cursor->key[cursor->size] = parent->key[rightSibling - 1];
			cursor->address[cursor->size] = parent->address[rightSibling - 1];
			for (int i = cursor->size + 1, j = 0; j < rightNode->size; j++)
			{
				cursor->key[i] = rightNode->key[j];
				cursor->address[i] = rightNode->address[j];
			}
			for (int i = cursor->size + 1, j = 0; j < rightNode->size; j++)
			{
				cursor->ptr[i] = rightNode->ptr[j];
				rightNode->ptr[j] = NULL;
			}
			cursor->size += rightNode->size + 1;
			rightNode->size = 0;
			//delete cursor
			removeInternal(parent->key[rightSibling - 1], parent->address[rightSibling - 1], parent, rightNode, bufferPool);
			numOfNode -= 1;
			numOfNodeDel += 1;
		}
	}

	Node* findParent(Node* cursor, Node* child)
	{
		//finds parent using depth first traversal and ignores leaf nodes as they cannot be parents
		//also ignores second last level because we will never find parent of a leaf node during insertion using this function
		Node* parent = NULL;
		if (cursor->IS_LEAF || (cursor->ptr[0])->IS_LEAF)
		{
			return NULL;
		}
		for (int i = 0; i < cursor->size + 1; i++)
		{
			if (cursor->ptr[i] == child)
			{
				parent = cursor;
				return parent;
			}
			else
			{
				parent = findParent(cursor->ptr[i], child);
				if (parent != NULL)return parent;
			}
		}
		return parent;
	}

	void search(int lowerBound, int upperBound)
	{
		bool search = true;
		int numOfIndexAccess = 1, numOfBlkAccess = 1, numOfMatch = 0;
		float totalRating = 0, avgRating = 0;
		vector <int> tempIndex;
		vector <string> tempData;
		vector <vector <int>> indexNode;
		vector <vector <string>> dataBlock;
		//search logic
		if (root == NULL)
		{
			//empty
			cout << "Tree is empty." << endl;
		}
		else
		{
			Node* cursor = root;
			//in the following while loop, cursor will travel to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					for (int j = 0; j < cursor->size; j++) 
					{
						if (indexNode.size() < 5)
							tempIndex.push_back(getRecords(cursor->address[j]).numVotes);
					}

					if (lowerBound <= cursor->key[i])
					{
						cursor = cursor->ptr[i];
						numOfIndexAccess += 1;
						break;
					}
					if (i == cursor->size - 1)
					{
						cursor = cursor->ptr[i + 1];
						numOfIndexAccess += 1;
						break;
					}
				}
				indexNode.push_back(tempIndex);
				tempIndex.clear();
			}
			Record record;
			//in the following for loop, we search for the key if it exists
			while (search) {
				for (int i = 0; i < cursor->size; i++)
				{
					record = getRecords(cursor->address[i]);
					if (indexNode.size() < 5)
						tempIndex.push_back(record.numVotes);
					if (dataBlock.size() < 5)
						tempData.push_back(record.tconst);

					if (i == cursor->size - 1 and search == true)
					{
						cursor = cursor->ptr[i + 1];
						numOfIndexAccess += 1;
						numOfBlkAccess += 1;
						break;
					}
					if (cursor->key[i] >= lowerBound and cursor->key[i] <= upperBound)
					{
						numOfMatch += 1;
						totalRating += record.avgRating;
					}
					else if (cursor->key[i] > upperBound)
					{
						search = false;
					}
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				if (dataBlock.size() < 5)
					dataBlock.push_back(tempData);
				tempIndex.clear();
				tempData.clear();
			}
			cout << "The content of the first 5 Index Nodes =" << endl;
			for (uint i = 0; i < indexNode.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < indexNode[i].size(); j++)
					cout << indexNode[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Index Nodes accessed = " << numOfIndexAccess << endl;

			cout << endl << "The content of the first 5 Data Blocks = " << endl;
			for (uint i = 0; i < dataBlock.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < dataBlock[i].size(); j++)
					cout << dataBlock[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Data Blocks accessed = " << numOfBlkAccess << endl << endl;
			
			cout << "The total number of matches = " << numOfMatch << endl;
			if (numOfMatch > 0)
				cout << "The average of 'averageRating' = " << totalRating / numOfMatch << endl;
			else
				cout << "No records found." << endl;
		}
	}

	Record getRecords(Address address) {
		void* mainMemoryAddress = (uchar*)address.blockAddress + address.offset;
		return (*(Record*)mainMemoryAddress);
	}
	void insert(Address address, int x)
	{
		//insert logic
		if (root == NULL)
		{
			root = new Node;
			root->key[0] = x;
			root->IS_LEAF = true;
			root->size = 1;
			root->address[0] = address;
			//increase number of node and tree level by 1
			numOfNode += 1;
			treeLvl += 1;
			//cout << "Created root\nInserted " << x << " successfully\n";
		}
		else
		{
			Node* cursor = root;
			Node* parent = NULL;
			//in the following while loop, cursor will travel to the leaf node possibly consisting the key
			while (cursor->IS_LEAF == false)
			{
				parent = cursor;
				for (int i = 0; i < cursor->size; i++)
				{
					if (x < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						break;
					}
					if (i == cursor->size - 1)
					{
						cursor = cursor->ptr[i + 1];
						break;
					}
				}
			}
			//now cursor is the leaf node in which we'll insert the new key
			if (cursor->size < MAX)
			{
				//if cursor is not full
				//find the correct position for new key
				int i = 0;
				while (x > cursor->key[i] && i < cursor->size) i++;
				//make space for new key
				for (int j = cursor->size;j > i; j--)
				{
					cursor->key[j] = cursor->key[j - 1];
					cursor->address[j] = cursor->address[j - 1];
				}
				cursor->key[i] = x;
				cursor->address[i] = address;
				cursor->size++;
				cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
				cursor->ptr[cursor->size - 1] = NULL;
			}
			else
			{
				//overflow condition
				//create new leaf node
				Node* newLeaf = new Node;
				//create a virtual node and insert x into it
				int virtualNode[MAX + 1];
				Address tempAddressList[MAX + 1];
				numOfNode += 1;

				for (int i = 0; i < MAX; i++)
				{
					virtualNode[i] = cursor->key[i];
					tempAddressList[i] = cursor->address[i];
				}
				int i = 0, j;
				while (x > virtualNode[i] && i < MAX) i++;
				//make space for new key
				for (int j = MAX + 1;j > i; j--)
				{
					virtualNode[j] = virtualNode[j - 1];
					tempAddressList[j] = tempAddressList[j - 1];
				}
				virtualNode[i] = x;
				tempAddressList[i] = address;
				newLeaf->IS_LEAF = true;
				//split the cursor into two leaf nodes
				cursor->size = (MAX + 1) / 2;
				newLeaf->size = MAX + 1 - ((MAX + 1) / 2);
				//make cursor point to new leaf node
				cursor->ptr[cursor->size] = newLeaf;
				//make new leaf node point to the next leaf node
				newLeaf->ptr[newLeaf->size] = cursor->ptr[MAX];
				cursor->ptr[MAX] = NULL;
				//now give elements to new leaf nodes
				for (i = 0; i < cursor->size; i++)
				{
					cursor->key[i] = virtualNode[i];
					cursor->address[i] = tempAddressList[i];
				}
				for (i = 0, j = cursor->size; i < newLeaf->size; i++, j++)
				{
					newLeaf->key[i] = virtualNode[j];
					newLeaf->address[i] = tempAddressList[j];
				}
				//modify the parent
				if (cursor == root)
				{
					//if cursor is a root node, we create a new root
					Node* newRoot = new Node;
					newRoot->key[0] = newLeaf->key[0];
					newRoot->address[0] = newLeaf->address[0];
					newRoot->ptr[0] = cursor;
					newRoot->ptr[1] = newLeaf;
					newRoot->IS_LEAF = false;
					newRoot->size = 1;
					root = newRoot;
					//increase number of node and tree level by 1
					numOfNode += 1;
					treeLvl += 1;
				}
				else
				{
					//insert new key in parent node
					insertInternal(newLeaf->key[0], newLeaf->address[0], parent, newLeaf);
				}
			}
		}
	}

	bool remove(int x, bufferPool *bufferPool)
	{
		//delete logic
		if (root == NULL)
		{
			cout << "Tree empty\n";
		}
		else
		{
			Node* cursor = root;
			Node* parent = NULL;
			int leftSibling, rightSibling;
			//in the following while loop, cursor will will travel to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					parent = cursor;
					leftSibling = i - 1; //leftSibling is the index of left sibling in the parent node
					rightSibling = i + 1; //rightSibling is the index of right sibling in the parent node
					if (x < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						break;
					}
					if (i == cursor->size - 1)
					{
						leftSibling = i;
						rightSibling = i + 2;
						cursor = cursor->ptr[i + 1];
						break;
					}
				}
			}
			//in the following for loop, we search for the key if it exists
			bool found = false;
			int pos;
			for (pos = 0; pos < cursor->size; pos++)
			{
				if (cursor->key[pos] == x)
				{
					found = true;
					bufferPool->deleteRecord(cursor->address[pos]);
					break;
				}
			}
			if (!found)//if key does not exist in that leaf node
			{
				return false;
			}
			//deleting the key
			for (int i = pos; i < cursor->size; i++)
			{
				cursor->key[i] = cursor->key[i + 1];
				cursor->address[i] = cursor->address[i + 1];
			}
			cursor->size--;
			if (cursor == root)//if it is root node, then make all pointers NULL
			{
				treeLvl -= 1;
				for (int i = 0; i < MAX + 1; i++)
				{
					cursor->ptr[i] = NULL;
				}
				if (cursor->size == 0)//if all keys are deleted
				{
					delete[] cursor->key;
					delete[] cursor->address;
					delete[] cursor->ptr;
					delete cursor;
					root = NULL;
				}
				return true;
			}
			cursor->ptr[cursor->size] = cursor->ptr[cursor->size + 1];
			cursor->ptr[cursor->size + 1] = NULL;
			if (cursor->size >= (MAX + 1) / 2)//no underflow
			{
				return true;
			}
			//underflow condition
			//first we try to transfer a key from sibling node
			//check if left sibling exists
			if (leftSibling >= 0)
			{
				Node* leftNode = parent->ptr[leftSibling];
				//check if it is possible to transfer
				if (leftNode->size >= (MAX + 1) / 2 + 1)
				{
					//make space for transfer
					for (int i = cursor->size; i > 0; i--)
					{
						cursor->key[i] = cursor->key[i - 1];
						cursor->address[i] = cursor->address[i - 1];
					}
					//shift pointer to next leaf
					cursor->size++;
					cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
					cursor->ptr[cursor->size - 1] = NULL;
					//transfer
					cursor->key[0] = leftNode->key[leftNode->size - 1];
					cursor->address[0] = leftNode->address[leftNode->size - 1];
					//shift pointer of leftsibling
					leftNode->size--;
					leftNode->ptr[leftNode->size] = cursor;
					leftNode->ptr[leftNode->size + 1] = NULL;
					//update parent
					parent->key[leftSibling] = cursor->key[0];
					parent->address[leftSibling] = cursor->address[0];
					return true;
				}
			}
			if (rightSibling <= parent->size)//check if right sibling exist
			{
				Node* rightNode = parent->ptr[rightSibling];
				//check if it is possible to transfer
				if (rightNode->size >= (MAX + 1) / 2 + 1)
				{
					//shift pointer to next leaf
					cursor->size++;
					cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
					cursor->ptr[cursor->size - 1] = NULL;
					//transfer
					cursor->key[cursor->size - 1] = rightNode->key[0];
					cursor->address[cursor->size - 1] = rightNode->address[0];
					//shift pointer of rightsibling
					rightNode->size--;
					rightNode->ptr[rightNode->size] = rightNode->ptr[rightNode->size + 1];
					rightNode->ptr[rightNode->size + 1] = NULL;
					//shift conent of right sibling
					for (int i = 0; i < rightNode->size; i++)
					{
						rightNode->key[i] = rightNode->key[i + 1];
						rightNode->address[i] = rightNode->address[i + 1];
					}
					//update parent
					parent->key[rightSibling - 1] = rightNode->key[0];
					parent->address[rightSibling - 1] = rightNode->address[0];
					return true;
				}
			}
			//must merge and delete a node
			if (leftSibling >= 0)//if left sibling exist
			{
				Node* leftNode = parent->ptr[leftSibling];
				// transfer all keys to leftsibling and then transfer pointer to next leaf node
				for (int i = leftNode->size, j = 0; j < cursor->size; i++, j++)
				{
					leftNode->key[i] = cursor->key[j];
					leftNode->address[i] = cursor->address[j];
				}
				//leftNode->ptr[leftNode->size] = NULL;
				leftNode->size += cursor->size;
				leftNode->ptr[leftNode->size] = cursor->ptr[cursor->size];
				removeInternal(parent->key[leftSibling], parent->address[leftSibling], parent, cursor, bufferPool);// delete parent node key
				numOfNode -= 1;
				numOfNodeDel += 1;
			}
			else if (rightSibling <= parent->size)//if right sibling exist
			{
				Node* rightNode = parent->ptr[rightSibling];
				// transfer all keys to cursor and then transfer pointer to next leaf node
				for (int i = cursor->size, j = 0; j < rightNode->size; i++, j++)
				{
					cursor->key[i] = rightNode->key[j];
					cursor->address[i] = rightNode->address[j];
				}
				//cursor->ptr[cursor->size] = NULL;
				cursor->size += rightNode->size;
				cursor->ptr[cursor->size] = rightNode->ptr[rightNode->size];
				removeInternal(parent->key[rightSibling - 1], parent->address[rightSibling - 1], parent, rightNode, bufferPool);// delete parent node key
				numOfNode -= 1;
				numOfNodeDel += 1;
			}
		}
		return true;
	}

	void display(Node* cursor, int level, int child)
	{
		if (cursor != NULL and level >= 0)
		{
			if (child == 0)
				cout << "Content of Root Node = ";
			else
				cout << "Content of " << child << " Child Node = ";

			for (int i = 0; i < cursor->size; i++)
			{ 
				cout << cursor->key[i] << " ";
			}
			cout << "\n";
			if (cursor->IS_LEAF != true)
			{
				for (int i = 0; i < cursor->size + 1; i++)
				{
					display(cursor->ptr[i], --level, ++child);
				}
			}
		}

		// depth first display
		//if (cursor != NULL)
		//{
		//	for (int i = 0; i < cursor->size; i++)
		//	{
		//		cout << cursor->key[i] << " ";
		//	}
		//	cout << "\n";
		//	if (cursor->IS_LEAF != true)
		//	{
		//		for (int i = 0; i < cursor->size + 1; i++)
		//		{
		//			display(cursor->ptr[i], level, child);
		//		}
		//	}
		//}
	}

	int getNumOfNode()
	{
		return numOfNode;
	}

	int getTreeLvl()
	{
		return treeLvl;
	}

	int getNumOfNodeDel()
	{
		return numOfNodeDel;
	}

	Node* getRoot()
	{
		return root;
	}

	void cleanUp(Node* cursor)
	{
		//clean up logic
		if (cursor != NULL)
		{
			if (cursor->IS_LEAF != true)
			{
				for (int i = 0; i < cursor->size + 1; i++)
				{
					cleanUp(cursor->ptr[i]);
				}
			}
			//for (int i = 0; i < cursor->size; i++)
			//{
			//	cout << "Deleted key from memory: " << cursor->key[i] << "\n";
			//}
			delete[] cursor->key;
			delete[] cursor->ptr;
			delete[] cursor->address;
			delete cursor;
		}
	}
};