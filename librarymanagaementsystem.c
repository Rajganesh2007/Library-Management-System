#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_BOOKS 200
#define MAX_USERS 100
#define MAX_TX 500
#define TEXT 80
#define DATA_FILE "library_data.dat"
#define FINE_PER_DAY 5

typedef struct {
    int id;
    char title[TEXT], author[TEXT], category[TEXT];
    int total, available, borrowedCount;
} Book;

typedef struct {
    int id;
    char name[TEXT], interest[TEXT];
} User;

typedef struct {
    int id, userId, bookId, active;
    time_t issueDate, dueDate, returnDate;
} Transaction;

Book books[MAX_BOOKS];
User users[MAX_USERS];
Transaction tx[MAX_TX];
int bookCount = 0, userCount = 0, txCount = 0;
int nextBookId = 1, nextUserId = 1, nextTxId = 1;

void readText(char *out, int size) {
    if (fgets(out, size, stdin)) {
        out[strcspn(out, "\n")] = '\0';
    } else out[0] = '\0';
}
void line(void) { puts("------------------------------------------------------------"); }

int containsIgnoreCase(const char *text, const char *key) {
    char a[TEXT * 2], b[TEXT * 2];
    int i;
    snprintf(a, sizeof(a), "%s", text);
    snprintf(b, sizeof(b), "%s", key);
    for (i = 0; a[i]; i++) a[i] = (char)tolower((unsigned char)a[i]);
    for (i = 0; b[i]; i++) b[i] = (char)tolower((unsigned char)b[i]);
    return strstr(a, b) != NULL;
}

int findBook(int id) {
    int i;
    for (i = 0; i < bookCount; i++) if (books[i].id == id) return i;
    return -1;
}
int findUser(int id) {
    int i;
    for (i = 0; i < userCount; i++) if (users[i].id == id) return i;
    return -1;
}

void saveData(void) {
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) { puts("Warning: could not save data."); return; }
    fwrite(&bookCount, sizeof(int), 1, f);
    fwrite(books, sizeof(Book), bookCount, f);
    fwrite(&userCount, sizeof(int), 1, f);
    fwrite(users, sizeof(User), userCount, f);
    fwrite(&txCount, sizeof(int), 1, f);
    fwrite(tx, sizeof(Transaction), txCount, f);
    fwrite(&nextBookId, sizeof(int), 1, f);
    fwrite(&nextUserId, sizeof(int), 1, f);
    fwrite(&nextTxId, sizeof(int), 1, f);
    fclose(f);
}
void loadData(void) {
    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;
    if (fread(&bookCount, sizeof(int), 1, f) != 1 || bookCount < 0 || bookCount > MAX_BOOKS) {
        fclose(f); bookCount = userCount = txCount = 0; return;
    }
    fread(books, sizeof(Book), bookCount, f);
    if (fread(&userCount, sizeof(int), 1, f) != 1 || userCount < 0 || userCount > MAX_USERS) {
        fclose(f); bookCount = userCount = txCount = 0; return;
    }
    fread(users, sizeof(User), userCount, f);
    if (fread(&txCount, sizeof(int), 1, f) != 1 || txCount < 0 || txCount > MAX_TX) {
        fclose(f); bookCount = userCount = txCount = 0; return;
    }
    fread(tx, sizeof(Transaction), txCount, f);
    fread(&nextBookId, sizeof(int), 1, f);
    fread(&nextUserId, sizeof(int), 1, f);
    fread(&nextTxId, sizeof(int), 1, f);
    fclose(f);
}

void seedData(void) {
    const char *titles[] = {"C Programming", "Data Structures in C", "Artificial Intelligence", "Machine Learning Basics", "Database Systems", "Operating Systems", "Clean Code", "Computer Networks"};
    const char *authors[] = {"Dennis Ritchie", "Reema Thareja", "Stuart Russell", "Andrew Ng", "Silberschatz", "Galvin", "Robert Martin", "Tanenbaum"};
    const char *cats[] = {"Programming", "Programming", "AI", "AI", "DBMS", "Operating Systems", "Programming", "Networking"};
    int i;
    for (i = 0; i < 8; i++) {
        books[i].id = nextBookId++;
        snprintf(books[i].title, TEXT, "%s", titles[i]);
        snprintf(books[i].author, TEXT, "%s", authors[i]);
        snprintf(books[i].category, TEXT, "%s", cats[i]);
        books[i].total = books[i].available = 3;
        books[i].borrowedCount = 0;
    }
    bookCount = 8;
    saveData();
}

void showBooks(void) {
    int i;
    line(); printf("%-4s %-29s %-20s %-18s %s\n", "ID", "TITLE", "AUTHOR", "CATEGORY", "AVAIL");
    line();
    for (i = 0; i < bookCount; i++)
        printf("%-4d %-29.29s %-20.20s %-18.18s %d/%d\n",
               books[i].id, books[i].title, books[i].author, books[i].category,
               books[i].available, books[i].total);
    if (!bookCount) puts("No books in library.");
    line();
}

void addBook(void) {
    Book b;
    if (bookCount >= MAX_BOOKS) { puts("Book storage full."); return; }
    b.id = nextBookId++;
    printf("Book title: "); readText(b.title, TEXT);
    printf("Author: "); readText(b.author, TEXT);
    printf("Category: "); readText(b.category, TEXT);
    printf("Number of copies: ");
    if (scanf("%d", &b.total) != 1 || b.total < 1) {
        while (getchar() != '\n');
        puts("Invalid number of copies."); return;
    }
    while (getchar() != '\n');
    b.available = b.total;
    b.borrowedCount = 0;
    books[bookCount++] = b;
    saveData();
    printf("Book added with ID %d.\n", b.id);
}

void searchBooks(void) {
    char key[TEXT];
    int i, found = 0, score;
    printf("Enter title, author, or category keyword: ");
    readText(key, TEXT);
    if (!key[0]) { puts("Search keyword cannot be empty."); return; }
    line();
    for (i = 0; i < bookCount; i++) {
        score = 0;
        if (containsIgnoreCase(books[i].title, key)) score += 3;
        if (containsIgnoreCase(books[i].author, key)) score += 2;
        if (containsIgnoreCase(books[i].category, key)) score += 1;
        if (score) {
            printf("Score %d | ID %d | %s | %s | %s | Available: %d\n",
                   score, books[i].id, books[i].title, books[i].author,
                   books[i].category, books[i].available);
            found = 1;
        }
    }
    if (!found) puts("No matching books found.");
    line();
}

void registerUser(void) {
    User u;
    if (userCount >= MAX_USERS) { puts("User storage full."); return; }
    u.id = nextUserId++;
    printf("User name: "); readText(u.name, TEXT);
    printf("Favourite category (e.g. AI, Programming, DBMS): ");
    readText(u.interest, TEXT);
    users[userCount++] = u;
    saveData();
    printf("User registered. User ID: %d\n", u.id);
}

void recommendBooks(void) {
    int uid, ui, i, best = -1, bestScore = -1;
    printf("Enter user ID: ");
    if (scanf("%d", &uid) != 1) { while (getchar() != '\n'); puts("Invalid input."); return; }
    while (getchar() != '\n');
    ui = findUser(uid);
    if (ui < 0) { puts("User not found. Register a user first."); return; }
    printf("\nRecommendations for %s (interest: %s):\n", users[ui].name, users[ui].interest);
    line();
    for (i = 0; i < bookCount; i++) {
        int score = 0;
        if (containsIgnoreCase(books[i].category, users[ui].interest)) score += 10;
        score += books[i].borrowedCount;
        if (books[i].available > 0) score += 1;
        printf("%2d points | %-29.29s | %-18.18s | Available: %d\n",
               score, books[i].title, books[i].category, books[i].available);
        if (score > bestScore) { bestScore = score; best = i; }
    }
    line();
    if (best >= 0)
        printf("Top pick: %s by %s (rule-based recommendation score: %d)\n",
               books[best].title, books[best].author, bestScore);
    puts("Note: this is a rule-based recommender, not a trained ML model.");
}

void issueBook(void) {
    int uid, bid, ui, bi, i;
    time_t now = time(NULL);
    if (txCount >= MAX_TX) { puts("Transaction storage full."); return; }
    printf("User ID: ");
    if (scanf("%d", &uid) != 1) { while (getchar() != '\n'); puts("Invalid input."); return; }
    printf("Book ID: ");
    if (scanf("%d", &bid) != 1) { while (getchar() != '\n'); puts("Invalid input."); return; }
    while (getchar() != '\n');
    ui = findUser(uid); bi = findBook(bid);
    if (ui < 0) { puts("User not found."); return; }
    if (bi < 0) { puts("Book not found."); return; }
    if (books[bi].available < 1) { puts("No copies available."); return; }
    for (i = 0; i < txCount; i++)
        if (tx[i].userId == uid && tx[i].bookId == bid && tx[i].active) {
            puts("This user already has this book issued."); return;
        }
    tx[txCount].id = nextTxId++;
    tx[txCount].userId = uid; tx[txCount].bookId = bid;
    tx[txCount].active = 1; tx[txCount].issueDate = now;
    tx[txCount].dueDate = now + 14 * 24 * 60 * 60;
    tx[txCount].returnDate = 0;
    txCount++;
    books[bi].available--;
    books[bi].borrowedCount++;
    saveData();
    printf("Book issued to %s. Due date: %s", users[ui].name, ctime(&tx[txCount-1].dueDate));
}

void returnBook(void) {
    int uid, bid, i, bi;
    time_t now = time(NULL);
    printf("User ID: ");
    if (scanf("%d", &uid) != 1) { while (getchar() != '\n'); puts("Invalid input."); return; }
    printf("Book ID: ");
    if (scanf("%d", &bid) != 1) { while (getchar() != '\n'); puts("Invalid input."); return; }
    while (getchar() != '\n');
    bi = findBook(bid);
    if (findUser(uid) < 0 || bi < 0) { puts("Invalid user or book ID."); return; }
    for (i = 0; i < txCount; i++) {
        if (tx[i].userId == uid && tx[i].bookId == bid && tx[i].active) {
            long lateDays = (long)(now - tx[i].dueDate) / (24 * 60 * 60);
            long fine = lateDays > 0 ? lateDays * FINE_PER_DAY : 0;
            tx[i].active = 0; tx[i].returnDate = now;
            books[bi].available++;
            saveData();
            printf("Book returned successfully. Fine: Rs. %ld\n", fine);
            return;
        }
    }
    puts("No active issue found for this user and book.");
}

void showActiveIssues(void) {
    int i, found = 0;
    time_t now = time(NULL);
    line(); puts("ACTIVE ISSUES");
    for (i = 0; i < txCount; i++) if (tx[i].active) {
        int ui = findUser(tx[i].userId), bi = findBook(tx[i].bookId);
        long days = (long)(now - tx[i].dueDate) / (24 * 60 * 60);
        printf("Tx %d | User: %s | Book: %s | Due: %s",
               tx[i].id, ui >= 0 ? users[ui].name : "Unknown",
               bi >= 0 ? books[bi].title : "Unknown", ctime(&tx[i].dueDate));
        if (days > 0) printf("   OVERDUE: %ld day(s), estimated fine Rs. %ld\n", days, days * FINE_PER_DAY);
        found = 1;
    }
    if (!found) puts("No active issues.");
    line();
}

void analytics(void) {
    int i, totalCopies = 0, available = 0, active = 0, best = -1;
    for (i = 0; i < bookCount; i++) {
        totalCopies += books[i].total;
        available += books[i].available;
        if (best < 0 || books[i].borrowedCount > books[best].borrowedCount) best = i;
    }
    for (i = 0; i < txCount; i++) if (tx[i].active) active++;
    line(); puts("LIBRARY ANALYTICS");
    printf("Unique book titles: %d\nTotal copies: %d\nAvailable copies: %d\nActive issues: %d\nRegistered users: %d\n",
           bookCount, totalCopies, available, active, userCount);
    if (best >= 0)
        printf("Most borrowed (tracked): %s (%d issues)\n", books[best].title, books[best].borrowedCount);
    line();
}

int main(void) {
    int choice;
    loadData();
    if (bookCount == 0) seedData();
    do {
        puts("\n========== SMART LIBRARY MANAGEMENT (C) ==========");
        puts("1. View all books");
        puts("2. Add a book");
        puts("3. Search books (ranked)");
        puts("4. Register user");
        puts("5. Recommend books");
        puts("6. Issue a book");
        puts("7. Return a book / calculate fine");
        puts("8. View active issues and overdue fines");
        puts("9. Library analytics");
        puts("0. Save and exit");
        printf("Choose: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            puts("Please enter a number."); continue;
        }
        while (getchar() != '\n');
        switch (choice) {
            case 1: showBooks(); break;
            case 2: addBook(); break;
            case 3: searchBooks(); break;
            case 4: registerUser(); break;
            case 5: recommendBooks(); break;
            case 6: issueBook(); break;
            case 7: returnBook(); break;
            case 8: showActiveIssues(); break;
            case 9: analytics(); break;
            case 0: saveData(); puts("Data saved. Goodbye!"); break;
            default: puts("Invalid choice.");
        }
    } while (choice != 0);
    return 0;
}
