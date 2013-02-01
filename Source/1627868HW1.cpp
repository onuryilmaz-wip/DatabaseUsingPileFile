// Onur Yilmaz


#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <string.h>

using namespace std;

// Definition of node for avail list
struct AvailListNode 
	{
	unsigned int offset;
	unsigned int size;	
	AvailListNode* next;
	AvailListNode* prev;	
	};
	
// Main function
int main(int argc, char** argv)
{	
	
	// Function declarations		
	int FieldNumber (fstream& in);
	AvailListNode* createNewNode (int OffsetInt, int SizeInt);
	void insertIntoAvailList(AvailListNode* TemporaryNode, AvailListNode* root);
	void writeAvailList( AvailListNode* root);
	int FindAndUpdateAvailList(AvailListNode* root, int size);
	int ReturnStartOfSearchedData(fstream& in, string SearchedID,int StartOfData);
	void InsertToDatabase (fstream& database_write, string ToInsert, int SizeToInsert, int WhereToInsert);
	void DeleteFromDatabase (fstream& database_write, int StartOfToBeDeleted);
	int SizeFromDatabase (fstream& in, int StartOfData)	;
	int DeletedOrNotInDatabase (fstream& in, int StartOfData);
	void WriteAvailListToDatabase(AvailListNode* root, int AvailListOffset, fstream& database_file_quit);
	void JoinCount(fstream& in,int StartOfData, fstream& check);
	void MergeAvail (AvailListNode* root);

	// Variable declarations
	int NumberOfFields;		// Number of fields in the main database
	int i; 			// Global counter
	string DatabaseFirstRow[100]; 	// String array for holding the first row of database
	int AVAIL_LIST_SIZE=164;	
	int AvailListOffset;		// Offset of available list from the beginning 
	int flag=1;			// Flag for quit command
	string command;			// String for reading commands
	string DatabaseName = argv[1];  // Database name
	
	// Open connection to database file
	fstream database_file;
	database_file.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);
	
	// Save number of fields in database
	NumberOfFields=FieldNumber(database_file);
	// cout << NumberOfFields << endl;
	
	// Read header of main database into string array
	database_file.seekg (4, ios::beg);	
	for(i=0;i<NumberOfFields; i++)	
	{
	getline(database_file,DatabaseFirstRow[i], '|');
	//cout<<DatabaseFirstRow[i]<<endl;
	}
	
	// Get the beginning of avail list
	AvailListOffset = database_file.tellg();
	database_file.seekg (AvailListOffset, ios::beg);
	
	// Linked List for "Available List"
	int NumberInAvailList; 		
	database_file.read(reinterpret_cast<char*>(&NumberInAvailList), sizeof(int));
	
	// Point to the first node in Avail List
	int FirstInAvailable = AvailListOffset+4;
 	database_file.seekg (FirstInAvailable, ios::beg);
	
	// Construct the linked list
	
	// Definition of dummy root
	AvailListNode* root;
	root = new AvailListNode;
	root->offset = 0;
	root->size = 10000;
	root-> next = NULL;
	root -> prev = NULL;
	
	// Start of data
	int StartOffsetOfData = AvailListOffset+AVAIL_LIST_SIZE;
	database_file.seekg (StartOffsetOfData, ios::beg);
	
	// If avail list is not empty
	if (NumberInAvailList != 0)	
	{	
		for(i=0; i<NumberInAvailList;i++)
		{
			
			// Read offset from database file
			database_file.seekg (FirstInAvailable+(4*2*i), ios::beg);
			int OffsetInt=0; 		
			database_file.read(reinterpret_cast<char*>(&OffsetInt), sizeof(int));
	
			// Read size from database file
			database_file.seekg (FirstInAvailable+(4*((2*i)+1)), ios::beg);		
			int SizeInt=0; 		
			database_file.read(reinterpret_cast<char*>(&SizeInt), sizeof(int));
	
			AvailListNode* TemporaryNode = createNewNode (OffsetInt, SizeInt);			
				
		
			if(OffsetInt!=0)
				insertIntoAvailList(TemporaryNode, root);	
		}
	}
	
	//writeAvailList(root);


	// Start of commands and operations
	while (flag)
	{
	cin >> command;


	// ---------------
	// INSERTION
	//----------------


	if (command == "insert")
	{	
		
		// Open connection to database file
		fstream database_file_insert;
		database_file_insert.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);
	
		
		// Read from input and save
		string ReadToInsert;
		string ReadToInsertTemp;
		getline(cin,ReadToInsert);
		string NonEmptySpaced;
		
		// Append '#'
		ReadToInsert = ReadToInsert + '#';
		// Erase the beginning empty space		
		NonEmptySpaced = ReadToInsert.substr(1,ReadToInsert.length());
		
		// Store the record size		
		int InsertRecordSize;
		InsertRecordSize = NonEmptySpaced.length();
		//cout << InsertRecordSize<<endl;		
		//cout << NonEmptySpaced<<endl;
	
		int WhereToInsertData;
	
		WhereToInsertData= FindAndUpdateAvailList(root, InsertRecordSize+5);
		//cout << WhereToInsertData << endl;
		
		InsertToDatabase(database_file_insert, NonEmptySpaced, InsertRecordSize+5, WhereToInsertData);
		//writeAvailList(root);
			
		database_file_insert.flush();
		database_file_insert.close();	
	}


	// ---------------
	// DELETION  
	//----------------

	else if (command == "delete")
	{
	
		// Read the record id for deletion
		string IDtoDelete;
		cin >> IDtoDelete;

		// Open connection to database file
		fstream database_file_delete;
		database_file_delete.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);	

		// Find the start offset of the data to be deleted 
		int StartOfToBeDeleted;
		StartOfToBeDeleted = ReturnStartOfSearchedData(database_file_delete, IDtoDelete,StartOffsetOfData);

		// Soft deletion
		if(StartOfToBeDeleted != -1)
		{	
			DeleteFromDatabase (database_file_delete, StartOfToBeDeleted);
		 
			// Inserting into avail list
			int SizeToAddAvail = SizeFromDatabase (database_file_delete, StartOfToBeDeleted);
			int OffsetToAddAvail = StartOfToBeDeleted;
			AvailListNode* DeletedAdded = createNewNode (OffsetToAddAvail,SizeToAddAvail);
			insertIntoAvailList(DeletedAdded, root);
			MergeAvail (root);
			MergeAvail (root);
			//writeAvailList(root);

			// Closing connections
			database_file_delete.flush();
			database_file_delete.close();	
		}
	}


	// ---------------
	// UPDATE 
	//----------------

	else if (command == "update")
{

		// Open connection to database file
		fstream database_file_update;
		database_file_update.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);
	
		
		// Read from input and save
		string ReadToUpdate;
		string ReadToUpdateTemp;
		getline(cin,ReadToUpdate);
		string NonEmptySpacedtoUpdate;
		
		// Append '#'
		ReadToUpdate = ReadToUpdate + '#';
		// Erase the beginning empty space		
		NonEmptySpacedtoUpdate = ReadToUpdate.substr(1,ReadToUpdate.length());
		
		// Store the record size		
		int UpdateRecordSize;
		UpdateRecordSize = NonEmptySpacedtoUpdate.length()+5;
		
		// Store the ID of updated
		string yeni = NonEmptySpacedtoUpdate;
		char* UpdateTemp = &yeni[0];
		string UpdateID;
		UpdateID = strtok(UpdateTemp, "|");
		
		// Locate the record
		
		// Open connection to database file
		fstream database_file_update_2;
		database_file_update_2.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);
		
		// Find offset of data
		int StartOfToBeUpdated;
		StartOfToBeUpdated = ReturnStartOfSearchedData(database_file_update, UpdateID,StartOffsetOfData);
		StartOfToBeUpdated = StartOfToBeUpdated;
		database_file_update_2.flush();
		database_file_update_2.close();			
		
		// Open connection to database file
		fstream database_file_update_3;
		database_file_update_3.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);	
		
		// Find size of data 
		int SizeOld =	SizeFromDatabase (database_file_update, StartOfToBeUpdated);	
		database_file_update_3.flush();
		database_file_update_3.close();	


		// Open connection to database file
		fstream database_file_update_4;
		database_file_update_4.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);	

		// If the size does not change after update		
		if(UpdateRecordSize == SizeOld) 
		{ 
			database_file_update_4.seekg(StartOfToBeUpdated+5);
			database_file_update_4<<NonEmptySpacedtoUpdate;
			database_file_update.flush();
			database_file_update.close();
		}
		
		// If the size increases with update
		else if(UpdateRecordSize > SizeOld) 
		{ 	
			// Open connection to database file
			fstream database_file_update_6;
			database_file_update_6.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);	
			DeleteFromDatabase (database_file_update_6, StartOfToBeUpdated);
			
			database_file_update_6.flush();
			database_file_update_6.close();
			
			// Inserting into avail list
			int SizeToAddAvail = SizeOld;
			int OffsetToAddAvail = StartOfToBeUpdated;
			AvailListNode* UpdatedAdded = createNewNode (OffsetToAddAvail,SizeToAddAvail);
			insertIntoAvailList(UpdatedAdded, root);
			MergeAvail (root);
			MergeAvail (root);
			
			int WhereToInsertUpdatedData;
			WhereToInsertUpdatedData = FindAndUpdateAvailList(root, UpdateRecordSize);
			//cout << WhereToInsertUpdatedData << endl;
	
			// Open connection to database file
			fstream database_file_update_5;
			database_file_update_5.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);	
			
			InsertToDatabase(database_file_update_5, NonEmptySpacedtoUpdate, UpdateRecordSize, WhereToInsertUpdatedData);
			database_file_update_5.flush();
			database_file_update_5.close();
			
			//writeAvailList(root);
		}

		// If the size decreases with update
		else if(UpdateRecordSize < SizeOld) 
		{ 	

			// Open connection to database file
			fstream database_file_update_7;
			database_file_update_7.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);	
			
			
			// Write to its original position
			InsertToDatabase (database_file_update_7, NonEmptySpacedtoUpdate, UpdateRecordSize, StartOfToBeUpdated);
			
			database_file_update_7.flush();
			database_file_update_7.close();		

			// Add remaining area into avail list
			int SizeToAddAvail = SizeOld-UpdateRecordSize;
			int OffsetToAddAvail = StartOfToBeUpdated+UpdateRecordSize;
			AvailListNode* UpdatedAdded = createNewNode (OffsetToAddAvail,SizeToAddAvail);
			insertIntoAvailList(UpdatedAdded, root);
			MergeAvail (root);
			MergeAvail (root);
			
			
			//writeAvailList(root);
		}

		//writeAvailList(root);
		database_file_update.flush();
		database_file_update.close();	

}	
	
	
	// ---------------
	// SELECTION  
	//----------------


	else if (command == "select")
	{
	
	// Read ID
	string IDtoSelect;
	cin >> IDtoSelect;

	// Open connection to database file
	fstream database_file_select;
	database_file_select.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);		

	// Locate the record
	int StartOfToBeSelected;
	StartOfToBeSelected = ReturnStartOfSearchedData(database_file_select, IDtoSelect,StartOffsetOfData);

	// Check the record is deleted
	int DeleteCheck = DeletedOrNotInDatabase (database_file_select, StartOfToBeSelected);
	// If record is not found
	if(StartOfToBeSelected == -1 || DeleteCheck == 1)
		cout << "Record not found" << endl;
	else 
	{	StartOfToBeSelected++;
		// Retrieve the fields
		database_file_select.seekg (StartOfToBeSelected+4, ios::beg);
		string complete;
		string PrintOut;
		getline(database_file_select,complete, '#');

		// Convert string into char array
		char TempArray[complete.size()];			
		for(int counter =0; counter < complete.size(); counter++)			
			TempArray[counter]=complete[counter];
		
		int counter = 0;
		
		// Match fields and printout
		for (int i=0; i<NumberOfFields;)
		{
			if(i==0) 
				cout << DatabaseFirstRow[i]<< " = ";
		
			while(counter < complete.size())
			{
				if (TempArray[counter] == '|') 
				{	
					i++;
					cout << endl;					
					cout << DatabaseFirstRow[i]<< " = ";
					counter++;
				}	
				cout << TempArray[counter];
				counter++;			
				if(i==NumberOfFields-1) i++;
				
			}	
		} 
		cout << endl;	
			
	}
	
	database_file_select.flush();
	database_file_select.close();	

}

	// ---------------
	// JOIN  
	//----------------


	else if (command == "join")
	{

	// Read path to join file
	string JoinFile;
	cin >>	JoinFile;

	// Open connection to database file
	fstream join_file;
	join_file.open (JoinFile.c_str(), ios::in | ios::out | ios::binary);

	// Save number of fields in database
	int NumberOfFieldsInJoin;
	NumberOfFieldsInJoin=FieldNumber(join_file);
	 //cout << NumberOfFieldsInJoin << endl;
	
	// Read header of main database into string array and locate ID
	string JoinFileFirstRow[100];
	int IDLocation;

	join_file.seekg (4, ios::beg);	
	
	for(i=0;i<NumberOfFieldsInJoin; i++)	
	{
		getline(join_file,JoinFileFirstRow[i], '|');
		if (JoinFileFirstRow[i] == DatabaseFirstRow[0]) 
			IDLocation = i+1;
			//cout<<JoinFileFirstRow[i]<<endl;
	}
	//cout << IDLocation<< endl;
	
	
	// Open connection to database file
	fstream database_file_join;
	database_file_join.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);		
	
	// Join Count function
	JoinCount(database_file_join, StartOffsetOfData, join_file);
	database_file_join.flush();
	database_file_join.close();	
	
	// Close connections
	join_file.flush();
	join_file.close();	

	
	}

	// This command is created for test purposes, it lists the content of avail list
	else if (command == "merge")
	{
	writeAvailList(root);
	}
	
	// ---------------
	// QUIT  
	//----------------

	else if (command == "quit")
	{
		// Open connection to database file
		fstream database_file_quit;
		database_file_quit.open (DatabaseName.c_str(), ios::in | ios::out | ios::binary);		
		
		// End of operations flag
		flag = 0;
		
		// Write avail list to main database file
		WriteAvailListToDatabase(root, AvailListOffset, database_file_quit);
		
		//writeAvailList(root);

		// Close connections
		database_file_quit.flush();
		database_file_quit.close();		 
	}
}

return 0;

}

// End of main function

// Function for returning the number of fields
int FieldNumber (fstream& in)
	{
		int temp_int;
		in.read(reinterpret_cast<char*>(&temp_int), sizeof(int));
		return temp_int;
	}

// Function for creating new node for Avail List
AvailListNode* createNewNode (int OffsetInt, int SizeInt)
	{
		// Allocate memory
		AvailListNode* TemporaryNode = (AvailListNode*)malloc(sizeof(AvailListNode));
		
		// Assign read values to new node
			TemporaryNode->offset = OffsetInt;
			TemporaryNode->size = SizeInt;
			TemporaryNode-> next = NULL;
			TemporaryNode->prev = NULL;
		return TemporaryNode;	
	}

// Function for finding and updating Avail List for a determined "size"
int FindAndUpdateAvailList(AvailListNode* root, int size)
{

	void insertIntoAvailList(AvailListNode* TemporaryNode, AvailListNode* root);
	int SizeSearched = size;
	AvailListNode* MoveOnList = root;
	
	// If Avail List is empty
	if (MoveOnList->next == NULL) 
		return -1;

	// If list is not empty
	MoveOnList = MoveOnList->next;
	do
	{	// Enough size is found
		if(MoveOnList->size >= SizeSearched )
			{	
			int SizeToAdd = MoveOnList->size - SizeSearched;
			int OffsetToAdd = MoveOnList->offset+SizeSearched;
			(MoveOnList->prev)->next = MoveOnList->next;
			
			// Deletion of node
			if ((MoveOnList->next) != NULL) 
				(MoveOnList->next)->prev = MoveOnList->prev;	
			
			// Insertion of new node
			if( SizeToAdd != 0)
			{
			AvailListNode* ToBeInserted = createNewNode(OffsetToAdd, SizeToAdd);
			insertIntoAvailList(ToBeInserted, root);	
			}
			return MoveOnList->offset; 
						}
	}	
	while	
		( (MoveOnList->next) != NULL);

return -1;

}

// Function for inserting new node to sorted linked list
void insertIntoAvailList(AvailListNode* TemporaryNode, AvailListNode* root)
{
	// Definition of a pointer for available list
	AvailListNode* MoveOnList = root;	

	// For the first insertion	
	if(root->next==NULL)
	{
		root->next = TemporaryNode;
		TemporaryNode->prev = root;
	}	
	// For the following insertions			
	else 
	{
		MoveOnList=root;
		// Find the location	
		while   (MoveOnList -> next != NULL && (MoveOnList->size) > (TemporaryNode->size)  )
				MoveOnList = MoveOnList->next;
				
		// Finalize the insertion				
		if (MoveOnList -> next == NULL && (MoveOnList->size) > (TemporaryNode->size) )
		{
		MoveOnList->next=TemporaryNode;	
		TemporaryNode->prev=MoveOnList;			
		}
		else 
		{
		TemporaryNode->next=MoveOnList;
		TemporaryNode->prev=MoveOnList->prev;			
		(MoveOnList->prev)->next = TemporaryNode;
		MoveOnList->prev=TemporaryNode;	
		}
	} 
}

// Function for writing out the Avail List
// For control and check
void writeAvailList( AvailListNode* root)
{
	AvailListNode* MoveOnList=root;
	do 
	{
	cout<<"size= "<< MoveOnList->size <<endl;
	cout<<"offset= "<< MoveOnList->offset <<endl;
	cout << "----------" << endl;
	MoveOnList=MoveOnList->next;
	}
	while(MoveOnList);	
			
}

// Function for returning the start of given record ID
int ReturnStartOfSearchedData(fstream& in, string SearchedID,int StartOfData)
{		
	char TempRead='1';
	int EndOfLast;
	int StartOfNext;	
	int FoundInData=1;
	string locatedID;
	int DeletedOrNotInDatabase (fstream& in, int StartOfData);

	// For the first record	
	in.seekg (StartOfData+5, ios::beg);
	getline(in,locatedID, '|');
	
	if(locatedID == SearchedID) 
	{
		return StartOfData;
	}

	// For next records		
	while (FoundInData && in)
	{
		in >> TempRead;	
		if (TempRead == '#')
		{
		EndOfLast=in.tellg();
		StartOfNext = EndOfLast+5;
		in.seekg(StartOfNext, ios::beg);
		getline(in,locatedID, '|');

		if(locatedID == SearchedID)
			{
			
			int DeleteCheck = DeletedOrNotInDatabase (in, EndOfLast);
			if (DeleteCheck == 0) 
				{
				FoundInData=0; 
				return EndOfLast;
				}
			}
		}
	}
// If not found
return -1;
}

// Function for inserting data into main database file
void InsertToDatabase (fstream& database_write, string ToInsert, int SizeToInsert, int WhereToInsert)
{
	// Insert to the end of file
	if(WhereToInsert == -1)	
	{
	database_write.seekg (0, ios::end);
	WhereToInsert = database_write.tellg();	
	}	
	
	// Write size and offset
	database_write.seekg (WhereToInsert, ios::beg);
	database_write.write ((char*)&SizeToInsert,4);
	database_write.seekg (WhereToInsert+4, ios::beg);
	// Write deleted flag and offset	
	int deleted = 0;
	database_write.write((char*)&deleted,1);
	database_write.seekg (WhereToInsert+5, ios::beg);
	// Write fields	
	database_write<<ToInsert;	
}

// Function for soft deletion in database file
void DeleteFromDatabase (fstream& database_write, int StartOfToBeDeleted)	
{
	if(StartOfToBeDeleted != -1) 
	{	database_write.seekp (StartOfToBeDeleted+4, ios::beg);
		int deleted = 1;
		database_write.write((char*)&deleted,1);
	}	
}

// Function for returning the size of record
int SizeFromDatabase (fstream& in, int StartOfData)	
{
		in.seekg (StartOfData, ios::beg);
		int SizeInt; 		
		in.read(reinterpret_cast<char*>(&SizeInt), sizeof(int));
		//cout << SizeInt<<endl;
		return SizeInt;		
}

// Function for checking whether record deleted in database file
// Returns 1 if data is deleted
int DeletedOrNotInDatabase (fstream& in, int StartOfData)	
{
		in.seekg (StartOfData+4, ios::beg);
		char DeletedChar=0;  
		int DeletedInt=0; 		
		in >> DeletedChar;
		DeletedInt=(int)DeletedChar;		
		return DeletedInt;			
	
}

// Function for writing Avail List into main database file
void WriteAvailListToDatabase(AvailListNode* root, int AvailListOffset, fstream& database_file_quit)
{
		// Pointer for writing the avail list		
		AvailListNode* Writer;
		Writer = root;
		int UpdatedNumberInAvail;
		
		// If list is empty
		if(root->next == NULL)
		{	UpdatedNumberInAvail=0;
			database_file_quit.seekg (AvailListOffset, ios::beg);
			database_file_quit.write ((char*)&UpdatedNumberInAvail,4);
		}
		
		else
		{	
			// Get the offset of first node
			database_file_quit.seekg (AvailListOffset+4, ios::beg);
			Writer = root->next;	
			UpdatedNumberInAvail=1;

			// Write all nodes
			do
			{	
			int WriteSize = Writer->size;
			int WriteOffset= Writer ->offset;
			database_file_quit.write ((char*)&WriteOffset,4);
			database_file_quit.write ((char*)&WriteSize,4);
			Writer = Writer->next;
			UpdatedNumberInAvail++;
			}
			while (Writer != NULL && UpdatedNumberInAvail<21);
		}
			// Write number of nodes in avail list
			database_file_quit.seekg (AvailListOffset, ios::beg);
			database_file_quit.write ((char*)&UpdatedNumberInAvail,4);	
	
			//writeAvailList(root);
}

// 1st function for printing out the number of counts in Join File
// It is used with JoinCounter function 
void JoinCount(fstream& in,int StartOfData, fstream& check)
{		
	char TempRead='1';
	int EndOfLast;
	int StartOfNext;	
	string locatedID;
	int DeletedOrNotInDatabase (fstream& in, int StartOfData);
	void JoinCounter(fstream& check,string ID);

	// For the first record	
	in.seekg (StartOfData+5, ios::beg);
	getline(in,locatedID, '|');
	
	cout << locatedID; 
	JoinCounter(check,locatedID);

	// For the next records		
	while (in)
	{
		in >> TempRead;	
		// If # is read
		if (TempRead == '#')
		{
		EndOfLast=in.tellg();
		StartOfNext = EndOfLast+5;
		in.seekg(StartOfNext, ios::beg);
		// Locate ID		
		getline(in,locatedID, '|');
		int DeleteCheck = DeletedOrNotInDatabase (in, EndOfLast);
				// If the record is not deleted			
				if (DeleteCheck == 0 && locatedID.size()>0) 
				{
				cout << locatedID;
				JoinCounter(check,locatedID);
				}
		}
	}
}

// 2nd function for actually counting the records in Join File
void JoinCounter(fstream& check,string ID)
{
	char* Temp = & ID[0];
	int IntegerID = atoi(Temp);
	int HashedValue = IntegerID % 20;
	int CountOfID=0;
	
	// Check sequential hashed area
	check.seekg(100+2408*HashedValue, ios::beg);
	size_t found;

	// Read number of records in this block
	int NumberInThisBlock=0; 		
	check.read(reinterpret_cast<char*>(&NumberInThisBlock), sizeof(int));
	int Start = check.tellg();
	
	// If blocs is not empty
	if(NumberInThisBlock)
	{	
		for(int i=0; i<24;i++)
		{
			string ReadComplete;
			getline(check, ReadComplete, '\0');
			found=ReadComplete.find(ID);
			// If ID is located in read part, increase counter
		  	if (found!=string::npos) 
				CountOfID++;
			// Find the next record in block			
			check.seekg(Start+100*(i+1), ios::beg);	
		} 
	// Read if Next Block is defined
	int NextBlock=0; 		
	check.read(reinterpret_cast<char*>(&NextBlock), sizeof(int));
		
		// As long as there are Next Blocks
		while(NextBlock!=0)
		{
			// Find the offset
			check.seekg(100+2408*NextBlock, ios::beg);
			// Read number in block			
			NumberInThisBlock=0; 	
			check.read(reinterpret_cast<char*>(&NumberInThisBlock), sizeof(int));
			Start = check.tellg();
			// If the block is not empty
			if(NumberInThisBlock)
			{
				for(int i=0; i<24;i++)
				{
					string ReadComplete;
					getline(check, ReadComplete, '\0');
					found=ReadComplete.find(ID);
					// If ID is located in read part, increase counter
				  	if (found!=string::npos) 
						CountOfID++;
					// Find the next record in block
					check.seekg(Start+100*(i+1), ios::beg);	
				} 
		
			}
		 	// Read if Next Block is defined	
			check.read(reinterpret_cast<char*>(&NextBlock), sizeof(int));
		
		}
	}
// Print out the counter for related ID
cout <<" "<<CountOfID << endl;
}

// Function for merging the adjacent empty slots in Avail List
void MergeAvail (AvailListNode* root) 
{
	// Define pointers
	AvailListNode* MoveOnList = root;
	AvailListNode* Merger = root;
	
	// If the avail list is not empty
	if (root->next != NULL) 
		MoveOnList = root->next;

	int total;

	do
	{	
		// Sum the offset and size 
		total = MoveOnList->size + MoveOnList->offset; 
		// First record after dummy root		
		Merger = root->next;
			
			do 
			{
			// If they are adjacent
			if (Merger->offset == total) 
				{
					int OffsetToAddAvail = MoveOnList->offset;
					int SizeToAddAvail = MoveOnList->size + Merger->size;
					// Delete first node
					(MoveOnList->prev)->next = MoveOnList->next;
					if(MoveOnList->next != NULL) (MoveOnList->next)->prev = MoveOnList->prev;
					// Delete second node
					(Merger->prev)->next = Merger->next;
					if(Merger->next != NULL) (Merger->next)->prev = Merger->prev;						
					// Insert new node
					AvailListNode* DeletedAdded = createNewNode (OffsetToAddAvail,SizeToAddAvail);
					insertIntoAvailList(DeletedAdded, root);
				}
			Merger = Merger->next;			
			}  
			while (Merger != NULL);						
						

	 MoveOnList=MoveOnList->next;

	} while (MoveOnList != NULL);
}

// End of code
