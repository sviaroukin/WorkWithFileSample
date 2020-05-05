#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>

using namespace std;

enum Role {
	Admin = 0,
	User
};

struct Account {
	string Login;
	string Password;
	Role Role;
};

struct AccountManager {

private:
	typedef void (*MenuFunc)(AccountManager* manager);
	//single instance of AccountManager
	bool isDisposed = false;
	static AccountManager* instance;

	//delete copy and move constructors to prevent creating instances by copying and moving the instance returned by GetInstance() method
	AccountManager(const AccountManager & source) = delete;
	AccountManager(AccountManager && source) = delete;

	AccountManager(string filePath, MenuFunc mainMenuFunc, MenuFunc userMenuFunc, MenuFunc adminMenuFunc) {
		this->filePath = filePath;
		this->mainMenuFunc = mainMenuFunc;
		this->userMenuFunc = userMenuFunc;
		this->adminMenuFunc = adminMenuFunc;

		accounts = new Account[size];
		InitAccountManager();
	}

public:
	string filePath;
	Account* accounts;
	Account* currentUserAccount = NULL;
	int count = 0;
	int size = 10;
	static void InitAccountManager(string filePath, MenuFunc mainMenuFunc, MenuFunc userMenuFunc, MenuFunc adminMenuFunc) {
		instance = new AccountManager(filePath, mainMenuFunc, userMenuFunc, adminMenuFunc);
	}

	static AccountManager* GetInstance() {
		return instance;
	}

	MenuFunc mainMenuFunc;
	MenuFunc userMenuFunc;
	MenuFunc adminMenuFunc;

	void InitAccountManager() {
		ifstream fin(filePath, ios::in);
		if (fin.is_open() && !isFileEmpty(fin))
		{
			while (!fin.eof()) {
				Account tmp;
				fin >> tmp.Login >> tmp.Password;
				int role;
				fin >> role;
				tmp.Role = static_cast<Role>(role);
				AddAccount(tmp);
			}
			fin.close();
			if (count > 0)
				return;
		}

		AddAccount("Admin", "admin", Admin);
	}

	bool isFileEmpty(ifstream& fin) {
		return fin.peek() == ifstream::traits_type::eof();
	}

	int FindAccount(string login) {
		for (int i = 0; i < count; i++) {
			if (accounts[i].Login == login) {
				return i;
			}
		}
		return -1;
	}

	void Menu() {
		if (currentUserAccount == NULL)
		{
			mainMenuFunc(this);
			return;
		}

		switch (currentUserAccount->Role) {
			case Admin: {
				adminMenuFunc(this);
				break;
			}
			case User: {
				userMenuFunc(this);
				break;
			}
		}
	}

	void Login(string login, string password) {
		int i = 0;
		while (i < count && accounts[i].Login != login) {
			i++;
		}

		if (i == count) {
			cout << "Error! Account wasn't found!" << endl;
		}else if (accounts[i].Password != password) {
			cout << "Error! Incorrect password!" << endl;
		}else currentUserAccount = accounts + i;

		Menu();
	}

	void Logout() {
		if (currentUserAccount == NULL) {
			cout << "You aren't logged in" << endl;
		}else currentUserAccount = NULL;

		Menu();
	}

	void DeleteAccount(string login) {
		int index = FindAccount(login);
		if (index == -1) {
			cout << "Account with login " << login << " wasn't found!" << endl;
			return;
		}

		for (int i = index; i < count - 1; i++) {
			accounts[i] = accounts[i + 1];
		}
		count--;
		cout << "Account was deleted!" << endl;
	}

	void EditAccount(string login, string password, Role role) {
		int index = FindAccount(login);
		if (index == -1) {
			cout << "Account with login " << login << " wasn't found!" << endl;
			return;
		}

		accounts[index].Password = password;
		accounts[index].Role = role;
		cout << "Account was edited!" << endl;
	}

	bool DoesAccountWithSameLoginExists(string login) {
		for (int i = 0; i < count; i++) {
			if (accounts[i].Login == login) {
				return true;
			}
		}
		return false;
	}

	void AddAccount(string login, string password, Role role) {
		Account newAccount;
		newAccount.Login = login;
		newAccount.Password = password;
		newAccount.Role = role;

		AddAccount(newAccount);
	}

	void AddAccount(Account account) {
		if (DoesAccountWithSameLoginExists(account.Login)) {
			cout << "Error! Account with login " << account.Login << " already exists!" << endl;
			return;
		}

		accounts[count] = account;
		count++;
		if (count == size) {
			size = size * 2;
			Account* newArr = new Account[size];
			for (int i = 0; i < count; i++) {
				newArr[i] = accounts[i];
			}
			delete accounts;
			accounts = newArr;
		}
	}

	void PrintAccounts() {
		for (int i = 0; i < count; i++) {
			cout << accounts[i].Login << " | " << accounts[i].Password << " | " << accounts->Role << endl;
		}
		cout << endl;
	}

	~AccountManager() {
		//prevent multiple call of destructor for already destructed(disposed) object
		if (!isDisposed) {
			ofstream fout(filePath);
			if (fout.is_open() && count > 0)
			{
				const char space = '\t';
				int i;
				for (i = 0; i < count - 1; i++) {
					fout << accounts[i].Login << space << accounts[i].Password << space << accounts[i].Role << endl;
				}
				fout << accounts[i].Login << space << accounts[i].Password << space << accounts[i].Role;
				fout.close();
			}
			delete[] accounts;
			isDisposed = true;
		}
	}
};

template<class T>
T EnterValue() {
	T value;
	while (!(cin >> value)) {
		cout << "Error! Try again!" << endl;
		cin.clear();
		cin.ignore(256, '\n');
	}
	return value;
}

void adminMenu(AccountManager* manager) {
	int exit = 0;
	while (!exit) {
		cout << "Menu:" << endl;
		cout << "1. Add user" << endl;
		cout << "2. Logout" << endl;
		int select = EnterValue<int>();
		switch (select)
		{
			case 1: {
				cout << "Enter login" << endl;
				string login = EnterValue<string>();
				cout << "Enter password" << endl;
				string password = EnterValue<string>();
				cout << "Enter role (0 - Admin, 1 - User)" << endl;
				Role role = static_cast<Role>(EnterValue<int>());
				manager->AddAccount(login, password, role);
				break;
			}
			case 2: {
				manager->Logout();
				exit = 1;
				break;
			}
			default: {
				cout << "There is no such menu point!" << endl;
				break;
			}
		}
	}
}

void userMenu(AccountManager* manager) {
	//someUserMenu
	int exit = 0;
	while (!exit) {
		cout << "Hello, " << manager->currentUserAccount->Login << endl;
		cout << "Menu:" << endl;
		cout << "1. Logout" << endl;
		int select = EnterValue<int>();
		switch (select)
		{
			case 1: {
				manager->Logout();
				exit = 1;
				break;
			}
			default: {
				cout << "There is no such menu point!" << endl;
				break;
			}
		}
	}
}

void mainMenu(AccountManager* manager) {
	int exit = 0;
	while (!exit) {
		cout << "Menu:" << endl;
		cout << "1. Login" << endl;
		cout << "2. Exit" << endl;
		int select = EnterValue<int>();
		switch (select)
		{
			case 1: {
				cout << "Enter login" << endl;
				string login = EnterValue<string>();
				cout << "Enter password" << endl;
				string password = EnterValue<string>();
				manager->Login(login, password);
				exit = 1;
				break;
			}
			case 2: {
				exit = 1;
				break;
			}
			default: {
				cout << "There is no such menu point!" << endl;
				break;
			}
		}
	}
}

//struct that guarantee that returned by static function GetAccountManager object is single in system
BOOL WINAPI ConsoleWindowEventsHandler(DWORD dwCtrlType) {

	//if console was closed
	if (CTRL_CLOSE_EVENT == dwCtrlType) {
		//delete single AccountManager instance
		AccountManager* manager = AccountManager::GetInstance();
		if (manager != NULL) {
			delete manager;
		}

		return TRUE;
	}

	return FALSE;
}

//definition for static struct variable
AccountManager* AccountManager::instance;

int main() {
	SetConsoleCtrlHandler(ConsoleWindowEventsHandler, true);

	string filePath = "Users.txt";
	AccountManager::InitAccountManager(filePath, mainMenu, userMenu, adminMenu);
	AccountManager* manager = AccountManager::GetInstance();
	manager->Menu();
	delete manager;

	return 0;
}