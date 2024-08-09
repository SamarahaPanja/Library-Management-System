#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;

// Utility class to generate unique IDs
class UniqueIDGenerator {
public:
    static int generateID(set<int>& existingIDs) {
        int id;
        do {
            id = rand() % 10000 + 1;
        } while (existingIDs.find(id) != existingIDs.end());
        existingIDs.insert(id);
        return id;
    }
};


// Abstract Member class
// Abstract Member class
class Member {
protected:
    int id;
    string name;
    int borrowLimit;
    int currentBorrowCount;  // New variable to track borrowed books

public:
    Member(int id, string name, int borrowLimit) 
        : id(id), name(name), borrowLimit(borrowLimit), currentBorrowCount(0) {}
    virtual ~Member() {}

    int getId() const { return id; }
    string getName() const { return name; }
    int getBorrowLimit() const { return borrowLimit; }
    int getCurrentBorrowCount() const { return currentBorrowCount; }

    virtual string getType() const = 0;

    bool canBorrow() const { return currentBorrowCount < borrowLimit; }

    virtual void borrowBook() {
        if (canBorrow()) {
            currentBorrowCount++;
        } else {
            cout << "Borrowing limit reached!\n";
        }
    }

    virtual void returnBook() {
        if (currentBorrowCount > 0) {
            currentBorrowCount--;
        }
    }

    virtual void save(ofstream &out) const {
        out << id << " " << name << " " << getType() << " " << currentBorrowCount << endl;
    }
};


// Derived Student class
class Student : public Member {
public:
    Student(int id, string name) : Member(id, name, 5) {}

    string getType() const override {
        return "Student";
    }
};

// Derived Professor class
class Professor : public Member {
public:
    Professor(int id, string name) : Member(id, name, 10) {}

    string getType() const override {
        return "Professor";
    }
};

// Derived TechnicalStaff class
class TechnicalStaff : public Member {
public:
    TechnicalStaff(int id, string name) : Member(id, name, 7) {}

    string getType() const override {
        return "TechnicalStaff";
    }
};


// Book class
// Book class
class Book {
private:
    int id;
    string title;
    string author;
    bool isBorrowed;
    int reservedBy;
    time_t reservationTime; // New attribute for reservation expiry

public:
    Book(int id, string title, string author) 
        : id(id), title(title), author(author), isBorrowed(false), reservedBy(-1), reservationTime(0) {}

    int getId() const { return id; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    bool getIsBorrowed() const { return isBorrowed; }

    void borrowBook() { 
        isBorrowed = true; 
        reservedBy = -1; 
        reservationTime = 0; 
    }

    void returnBook() { 
        isBorrowed = false; 
    }

    bool isReserved() const { return reservedBy != -1; }
    void reserveBook(int memberId) { 
        reservedBy = memberId; 
        reservationTime = time(0); 
    }

    int getReservedBy() const { return reservedBy; }

    void checkReservationExpiry() {
        if (isReserved()) {
            time_t now = time(0);
            double secondsElapsed = difftime(now, reservationTime);
            if (secondsElapsed > 86400) { // 24 hours = 86400 seconds
                reservedBy = -1;
                reservationTime = 0;
            }
        }
    }

    void save(ofstream &out) const {
        out << id << " " << title << " " << author << " " << isBorrowed << " " << reservedBy << " " << reservationTime << endl;
    }
};


// Transaction class
// Transaction class
class Transaction {
private:
    string date;
    int bookId;
    int memberId;
    string type;

public:
    Transaction(int bookId, int memberId, string type) 
        : bookId(bookId), memberId(memberId), type(type) {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        stringstream ss;
        ss << 1900 + ltm->tm_year << "-" 
           << setfill('0') << setw(2) << 1 + ltm->tm_mon << "-"
           << setfill('0') << setw(2) << ltm->tm_mday;
        date = ss.str();
    }

    void save(ofstream &out) const {
        out << date << " " << bookId << " " << memberId << " " << type << endl;
    }
};

// Library class
class Library {
private:
    vector<Book> books;
    vector<Member*> members;
    set<int> bookIDs;
    set<int> memberIDs;
    vector<Transaction> transactions;

    // Find book by title and author

    void loadBooks() {
        ifstream bookFile("books.txt");

        int id, reservedBy;
        string title, author;
        bool isBorrowed;
        time_t reservationTime;

        while (bookFile >> id >> title >> author >> isBorrowed >> reservedBy >> reservationTime) {
            books.emplace_back(id, title, author);
            if (isBorrowed) {
                books.back().borrowBook();
            }
            if (reservedBy != -1) {
                books.back().reserveBook(reservedBy);
                books.back().reservationTime = reservationTime; // Load reservation time
            }
        }

        bookFile.close();
    }

    // Load members from file
    void loadMembers() {
        ifstream memberFile("members.txt");

        int id, borrowCount;
        string name, type;

        while (memberFile >> id >> name >> type >> borrowCount) {
            Member* member = nullptr;
            if (type == "Student") {
                member = new Student(id, name);
            } else if (type == "Professor") {
                member = new Professor(id, name);
            } else if (type == "TechnicalStaff") {
                member = new TechnicalStaff(id, name);
            }

            if (member) {
                members.push_back(member);
                while (member->getCurrentBorrowCount() < borrowCount) {
                    member->borrowBook();
                }
            }
        }

        memberFile.close();
    }

    Book* findBookByTitleAuthor(string title, string author) {
        for (auto& book : books) {
            if (book.getTitle() == title && book.getAuthor() == author) {
                return &book;
            }
        }
        return nullptr;
    }

    // Find member by ID
    Member* findMember(int memberId) {
        for (auto& member : members) {
            if (member->getId() == memberId) {
                return member;
            }
        }
        return nullptr;
    }

    // Find book by ID
    Book* findBookById(int bookId) {
        for (auto& book : books) {
            if (book.getId() == bookId) {
                return &book;
            }
        }
        return nullptr;
    }

    // Save changes to book data
    void updateBookInFile(const Book& updatedBook) {
        ifstream infile("books.txt");
        ofstream outfile("temp.txt");

        int id, reservedBy;
        string title, author;
        bool isBorrowed;
        time_t reservationTime;

        while (infile >> id >> title >> author >> isBorrowed >> reservedBy >> reservationTime) {
            if (id == updatedBook.getId()) {
                updatedBook.save(outfile);
            } else {
                outfile << id << " " << title << " " << author << " " << isBorrowed << " " << reservedBy << " " << reservationTime << endl;
            }
        }

        infile.close();
        outfile.close();

        remove("books.txt");
        rename("temp.txt", "books.txt");
    }

    // Save changes to member data
    void updateMemberInFile(const Member& updatedMember) {
        ifstream infile("members.txt");
        ofstream outfile("temp.txt");

        int id;
        string name, type;

        while (infile >> id >> name >> type) {
            if (id == updatedMember.getId()) {
                updatedMember.save(outfile);
            } else {
                outfile << id << " " << name << " " << type << endl;
            }
        }

        infile.close();
        outfile.close();

        remove("members.txt");
        rename("temp.txt", "members.txt");
    }

public:
    Library() {
        loadBooks();
        loadMembers();
    }

    ~Library() {
        for (auto& member : members) {
            delete member;
        }
        members.clear();
    }

    void addBook(string title, string author) {
        int id = UniqueIDGenerator::generateID(bookIDs);
        Book newBook(id, title, author);
        books.push_back(newBook);

        ofstream bookFile("books.txt", ios::app);
        newBook.save(bookFile);
        bookFile.close();

        cout << "Book added successfully.\n";
    }

    void addMember(Member* member) {
        members.push_back(member);
        memberIDs.insert(member->getId());

        ofstream memberFile("members.txt", ios::app);
        member->save(memberFile);
        memberFile.close();

        cout << "Member added successfully.\n";
    }

     // Borrow a book by title and author for a specific member
void borrowBook(int memberId, string title, string author) {
    Member* member = findMember(memberId);

    if (!member) {
        cout << "Member not found!" << endl;
        return;
    }

    if (!member->canBorrow()) {
        cout << "Member has reached their borrowing limit!" << endl;
        return;
    }

    bool bookBorrowed = false;
    bool reservationOptionProvided = false;

    for (auto& book : books) {
        if (book.getTitle() == title && book.getAuthor() == author) {
            book.checkReservationExpiry(); // Check reservation expiry for each copy

            // If the book is available and not reserved, or reserved by this member
            if (!book.getIsBorrowed()) {
                if (!book.isReserved() || book.getReservedBy() == memberId) {
                    // Proceed to borrow the book
                    book.borrowBook();
                    member->borrowBook();
                    updateBookInFile(book);
                    updateMemberInFile(*member);

                    transactions.emplace_back(book.getId(), member->getId(), "Borrow");
                    ofstream transactionFile("transactions.txt", ios::app);
                    transactions.back().save(transactionFile);
                    transactionFile.close();

                    cout << "Book borrowed successfully by " << member->getName() << "." << endl;
                    bookBorrowed = true;
                    break; // Exit loop after borrowing the book
                }
            }
        }
    }

    if (!bookBorrowed) {
        // If no available copies, prompt for reservation
        cout << "No available copies of the book \"" << title << "\" by " << author << " were found." << endl;
        cout << "Would you like to reserve a copy for future borrowing? (yes/no): ";
        
        string userResponse;
        cin >> userResponse;

        if (userResponse == "yes") {
            bool reserved = false;
            for (auto& book : books) {
                if (book.getTitle() == title && book.getAuthor() == author && !book.isReserved() && !book.getIsBorrowed()) {
                    book.reserveBook(memberId);
                    updateBookInFile(book);
                    cout << "Book reserved successfully by " << member->getName() << "." << endl;
                    reserved = true;
                    break;
                }
            }
            if (!reserved) {
                cout << "All copies of the book are currently reserved or borrowed." << endl;
            }
        } else {
            cout << "Reservation canceled." << endl;
        }
    }
}

    // Return a book by ID for a specific member
    void returnBook(int bookId, int memberId) {
        Book* book = findBookById(bookId);
        Member* member = findMember(memberId);

        if (!book) {
            cout << "Book not found!" << endl;
            return;
        }

        if (!member) {
            cout << "Member not found!" << endl;
            return;
        }

        if (!book->getIsBorrowed()) {
            cout << "Book is not currently borrowed!" << endl;
            return;
        }

        book->returnBook();
        member->returnBook();
        updateBookInFile(*book);
        updateMemberInFile(*member);

        transactions.emplace_back(book->getId(), member->getId(), "Return");
        ofstream transactionFile("transactions.txt", ios::app);
        transactions.back().save(transactionFile);
        transactionFile.close();

        cout << "Book returned successfully by " << member->getName() << endl;
    }

    // Reserve a book by title and author for a specific member
   
    void saveAllData() {
        ofstream bookFile("books.txt");
        for (const auto& book : books) {
            book.save(bookFile);
        }
        bookFile.close();

        ofstream memberFile("members.txt");
        for (const auto& member : members) {
            member->save(memberFile);
        }
        memberFile.close();

        ofstream transactionFile("transactions.txt");
        for (const auto& transaction : transactions) {
            transaction.save(transactionFile);
        }
        transactionFile.close();
    }

    void showAllBooks() const {
        cout << "\nBooks in the library:\n";
        for (const auto& book : books) {
            cout << "ID: " << book.getId() << ", Title: " << book.getTitle() << ", Author: " << book.getAuthor() << endl;
        }
    }

    void showAllMembers() const {
        cout << "\nMembers in the library:\n";
        for (const auto& member : members) {
            cout << "ID: " << member->getId() << ", Name: " << member->getName() << ", Type: " << member->getType() << endl;
            member->showBorrowedBooks();
        }
    }

    void menu() {
        int choice;
        do {
            cout << "\nLibrary Management System Menu:\n";
            cout << "1. Add Book\n";
            cout << "2. Add Member\n";
            cout << "3. Borrow Book\n";
            cout << "4. Return Book\n";
            cout << "5. Show All Books\n";
            cout << "6. Show All Members\n";
            cout << "7. Save and Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
                case 1: {
                    string title, author;
                    cout << "Enter book title: ";
                    cin.ignore(); 
                    getline(cin, title);
                    cout << "Enter book author: ";
                    getline(cin, author);
                    addBook(title, author);
                    break;
                }
                case 2: {
                    int memberType;
                    string name;
                    cout << "Enter member name: ";
                    cin.ignore();
                    getline(cin, name);
                    cout << "Select member type:\n";
                    cout << "1. Student\n";
                    cout << "2. Professor\n";
                    cout << "3. Technical Staff\n";
                    cout << "Enter your choice: ";
                    cin >> memberType;

                    int memberId = UniqueIDGenerator::generateID(memberIDs);

                    if (memberType == 1) {
                        addMember(new Student(memberId, name));
                    } else if (memberType == 2) {
                        addMember(new Professor(memberId, name));
                    } else if (memberType == 3) {
                        addMember(new TechnicalStaff(memberId, name));
                    } else {
                        cout << "Invalid member type selected.\n";
                    }
                    break;
                }
                case 3: {
                    int memberId;
                    string title, author;
                    cout << "Enter member ID: ";
                    cin >> memberId;
                    cout << "Enter book title: ";
                    cin.ignore();
                    getline(cin, title);
                    cout << "Enter book author: ";
                    getline(cin, author);
                    borrowBook(memberId, title, author);
                    break;
                }
                case 4: {
                    int memberId, bookId;
                    cout << "Enter member ID: ";
                    cin >> memberId;
                    cout << "Enter book ID: ";
                    cin >> bookId;
                    returnBook(memberId, bookId);
                    break;
                }
                case 5: {
                    showAllBooks();
                    break;
                }
                case 6: {
                    showAllMembers();
                    break;
                }
                case 7: {
                    cout << "Exiting the library...\n";
                    break;
                }
                default:
                    cout << "Invalid choice. Please try again.\n";
            }
        } while (choice != 7);
    }
};


int main() {
    Library library;
    library.menu();
    return 0;
}
