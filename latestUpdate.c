#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<time.h>
#include "md5.h"

#define MAX_MEDICINE 250
#define MAX_CART 50
#define PAGE_SIZE 20

struct Medicine{
    int id;
    char name[30];
    float price;
    int qty;
    char company[30];
    char category[30];
    char purchasedPrice[35];
};

struct CartItem{
    int id;
    char name[30];
    float price;
    int qty;
};

struct Customer{
    int id;
    char name[30];
    char phone[15];
    char location[30];
    int points;
};

struct Purchase{
    int id;
    int customerId;
    char customerName[30];
    char phone[15];
    float subtotal;
    float discount;
    float tax;
    float total;
    float profit;
    char date[15]; 
};

struct Shop{
    int id;
    char name[30];
    char username[20];
    char password[40];
    char folderPath[50];
};

int currentShopId = 0;
char currentShopFolder[50] = "";

struct DecryptCacheItem {
    char hash[35];
    float price;
};
struct DecryptCacheItem decryptCache[256];
int decryptCacheCount = 0;

int getIntegerInput(int *output) {
    if (scanf("%d", output) != 1) {
        while (getchar() != '\n');
        return 0;
    }
    return 1;
}

int readMedicines(struct Medicine *medList);
void writeMedicines(struct Medicine *medList, int count);
float decryptPrice(const char *hash);
int findCustomerByPhone(const char *phone, struct Customer *cust);

void addMedicine();
void viewMedicine();
void searchMedicine();
void updateMedicine();
void deleteMedicine();
void customerCheckout(int customerId);

void registerCustomer();
int customerLogin();
void customerDashboard(int customerId);
void viewCustomerProfile(int customerId);
void viewPreviousPurchases(int customerId);
void checkLoyaltyPoints(int customerId);
void addLoyaltyPoints(int customerId, float total);

int getNextCustomerId();
int getNextPurchaseId();
int findCustomerById(int customerId, struct Customer *cust);
void updateCustomerRecord(struct Customer cust);

void normalizeName(char *str);
void registerShop();
int shopLogin();
void getShopFilePath(char *filename, char *outputBuffer);
void syncMissingMedicines();
void viewMissingMedicines();
void requestMissingMedicine(int customerId);
void adminSalesDashboard();
int getShopPassword(int shopId, char *passwordBuffer);
int getNextShopId();
void guestDashboard();


int readMedicines(struct Medicine *medList) {
    char path[100];
    getShopFilePath("medicine.txt", path);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return 0;
    
    int count = 0;
    while (count < MAX_MEDICINE && 
           fscanf(fp, "%d %s %f %d %s %s %s", 
                  &medList[count].id, medList[count].name, &medList[count].price, &medList[count].qty, 
                  medList[count].company, medList[count].category, medList[count].purchasedPrice) == 7) {
        count++;
    }
    fclose(fp);
    return count;
}

void writeMedicines(struct Medicine *medList, int count) {
    char path[100];
    getShopFilePath("medicine.txt", path);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d %s %.2f %d %s %s %s\n", 
                medList[i].id, medList[i].name, medList[i].price, medList[i].qty, 
                medList[i].company, medList[i].category, medList[i].purchasedPrice);
    }
    fclose(fp);
}

float decryptPrice(const char *hash) {
    for (int i = 0; i < decryptCacheCount; i++) {
        if (strcmp(decryptCache[i].hash, hash) == 0) {
            return decryptCache[i].price;
        }
    }

    char temp[32];
    char testHash[40];
    for (int i = 1; i <= 5000000; i++) {
        float val = i / 100.0f;
        int integer_part = i / 100;
        int fractional_part = i % 100;
        sprintf(temp, "%d.%02d", integer_part, fractional_part);

        computeMD5(temp, testHash);
        if (strcmp(hash, testHash) == 0) {
            if (decryptCacheCount < 256) {
                strcpy(decryptCache[decryptCacheCount].hash, hash);
                decryptCache[decryptCacheCount].price = val;
                decryptCacheCount++;
            }
            return val;
        }
    }
    return 0.0f;
}

int findCustomerByPhone(const char *phone, struct Customer *cust) {
    char path[100];
    getShopFilePath("customer.txt", path);
    FILE *fp = fopen(path, "r");
    struct Customer c;

    if (fp == NULL) {
        return 0;
    }

    while (fscanf(fp, "%d %s %s %s %d", &c.id, c.name, c.phone, c.location, &c.points) == 5) {
        if (strcmp(c.phone, phone) == 0) {
            *cust = c;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void normalizeName(char *str){
    int i = 0;
    if(str[0] == '\0') return;
    str[0] = toupper(str[0]);
    for(i = 1; str[i] != '\0'; i++){
        str[i] = tolower(str[i]);
    }
}

void getShopFilePath(char *filename, char *outputBuffer){
    if(strlen(currentShopFolder) > 0){
        sprintf(outputBuffer, "%s/%s", currentShopFolder, filename);
    }else{
        strcpy(outputBuffer, filename);
    }
}

int getNextShopId(){
    FILE *fp = fopen("shops.txt", "r");
    struct Shop s;
    int maxId = 0;
    if(fp == NULL) return 1;
    while(fscanf(fp, "%d %s %s %s %s", &s.id, s.name, s.username, s.password, s.folderPath) == 5){
        if(s.id > maxId) maxId = s.id;
    }
    fclose(fp);
    return maxId + 1;
}

void registerShop(){
    FILE *fp = fopen("shops.txt", "a");
    struct Shop s;
    char mkdirCmd[100];

    if(fp == NULL){
        printf("\nUnable to Open Shop File!\n");
        return;
    }

    s.id = getNextShopId();
    printf("\nEnter Shop Name: ");
    scanf("%s", s.name);
    printf("Enter Shop Username: ");
    scanf("%s", s.username);
    printf("Enter Shop Password: ");
    char tempPass[40];
    scanf("%s", tempPass);
    computeMD5(tempPass, s.password);

    sprintf(s.folderPath, "shop_%d", s.id);

#ifdef _WIN32
    sprintf(mkdirCmd, "mkdir %s 2> NUL", s.folderPath);
#else
    sprintf(mkdirCmd, "mkdir -p %s", s.folderPath);
#endif
    system(mkdirCmd);

    fprintf(fp, "%d %s %s %s %s\n", s.id, s.name, s.username, s.password, s.folderPath);
    fclose(fp);

    printf("\nShop Registered Successfully!\n");
    printf("Shop ID: %d | Dedicated Workspace Created: %s/\n", s.id, s.folderPath);
}

int getShopPassword(int shopId, char *passwordBuffer){
    FILE *fp = fopen("shops.txt", "r");
    struct Shop s;
    if(fp == NULL) return 0;
    while(fscanf(fp, "%d %s %s %s %s", &s.id, s.name, s.username, s.password, s.folderPath) == 5){
        if(s.id == shopId){
            strcpy(passwordBuffer, s.password);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int shopLogin(){
    FILE *fp = fopen("shops.txt", "r");
    struct Shop s;
    char user[20], pass[20];
    char hashedPass[40];

    if(fp == NULL){
        printf("\nNo Registered Shops Found! Please Register First.\n");
        return 0;
    }

    printf("\n--- Shop Admin Login ---\n");
    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);
    computeMD5(pass, hashedPass);

    while(fscanf(fp, "%d %s %s %s %s", &s.id, s.name, s.username, s.password, s.folderPath) == 5){
        if(strcmp(user, s.username) == 0 && strcmp(hashedPass, s.password) == 0){
            if(currentShopId != 0 && s.id != currentShopId){
                printf("\nError: You cannot login to a different shop (%s) in this session.\n", s.name);
                printf("Please perform a System Logout from the Admin Menu first.\n");
                fclose(fp);
                return 0;
            }
            currentShopId = s.id;
            strcpy(currentShopFolder, s.folderPath);
            fclose(fp);
            printf("\nLogin Successful! Welcome to Shop: %s\n", s.name);
            return 1;
        }
    }

    fclose(fp);
    printf("\nInvalid Shop Username or Password!\n");
    return 0;
}

int getNextCustomerId(){
    char path[100];
    getShopFilePath("customer.txt", path);
    FILE *fp=fopen(path,"r");
    struct Customer c;
    int maxId=1000;

    if(fp == NULL){
        return 1001;
    }

    while(fscanf(fp,"%d %s %s %s %d",&c.id,c.name,c.phone,c.location,&c.points)==5){
        if(c.id > maxId){
            maxId = c.id;
        }
    }
    fclose(fp);

    return maxId + 1;
}

int getNextPurchaseId(){
    char path[100];
    getShopFilePath("purchase.txt", path);
    FILE *fp = fopen(path, "r");
    struct Purchase p;
    int maxId = 0;

    if(fp == NULL) return 1;

    while(fscanf(fp, "%d %d %s %s %f %f %f %f %f %s", 
                 &p.id, &p.customerId, p.customerName, p.phone, 
                 &p.subtotal, &p.discount, &p.tax, &p.total, &p.profit, p.date) == 10){
        if (p.id > maxId) {
            maxId = p.id;
        }
    }
    fclose(fp);
    return maxId + 1;
}

int findCustomerById(int customerId, struct Customer *cust){
    char path[100];
    getShopFilePath("customer.txt", path);
    FILE *fp=fopen(path,"r");
    struct Customer c;

    if(fp == NULL){
        return 0;
    }

    while(fscanf(fp,"%d %s %s %s %d",&c.id,c.name,c.phone,c.location,&c.points)==5){
        if(c.id == customerId){
            *cust = c;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);

    return 0;
}

void updateCustomerRecord(struct Customer cust){
    char path[100], tempPath[100];
    getShopFilePath("customer.txt", path);
    getShopFilePath("temp_customer.txt", tempPath);

    FILE *fp=fopen(path,"r");
    FILE *temp=fopen(tempPath,"w");
    struct Customer c;

    if(fp == NULL || temp == NULL){
        if(fp != NULL) fclose(fp);
        if(temp != NULL) fclose(temp);
        return;
    }

    while(fscanf(fp,"%d %s %s %s %d",&c.id,c.name,c.phone,c.location,&c.points)==5){
        if(c.id == cust.id){
            fprintf(temp,"%d %s %s %s %d\n",cust.id,cust.name,cust.phone,cust.location,cust.points);
        }else{
            fprintf(temp,"%d %s %s %s %d\n",c.id,c.name,c.phone,c.location,c.points);
        }
    }

    fclose(fp);
    fclose(temp);
    remove(path);
    rename(tempPath,path);
}

void registerCustomer(){
    char path[100];
    getShopFilePath("customer.txt", path);
    struct Customer temp;
    char phone[15];

    printf("\nEnter Phone Number: ");
    scanf("%s", phone);

    if(findCustomerByPhone(phone, &temp)){
        printf("\nError: This Phone Number is already registered!\n");
        return;
    }

    FILE *fp=fopen(path,"a");
    struct Customer c;

    if(fp == NULL){
        printf("\nUnable to Open Customer File!\n");
        return;
    }

    c.id = getNextCustomerId();
    c.points = 0;
    strcpy(c.phone, phone);

    printf("Enter Customer Name: ");
    scanf("%s",c.name);
    printf("Enter Location Name: ");
    scanf("%s",c.location);

    fprintf(fp,"%d %s %s %s %d\n",c.id,c.name,c.phone,c.location,c.points);
    fclose(fp);

    printf("\nSignup Successful!\n");
    printf("Your Customer ID is: %d\n",c.id);
    printf("Use your Phone Number (%s) for future login.\n", c.phone);
}

int customerLogin(){
    char phone[15];
    struct Customer c;

    printf("\nEnter Customer Phone Number: ");
    scanf("%s", phone);

    if(findCustomerByPhone(phone, &c)){
        printf("\nLogin Successful! Welcome %s\n",c.name);
        return c.id;
    }

    printf("\nInvalid Phone Number!\n");
    return 0;
}

void viewCustomerProfile(int customerId){
    struct Customer c;

    if(findCustomerById(customerId,&c)){
        printf("\n=== Customer Profile ===\n");
        printf("Customer ID: %d\n",c.id);
        printf("Name: %s\n",c.name);
        printf("Phone: %s\n",c.phone);
        printf("Location: %s\n",c.location);
        printf("Loyalty Points: %d\n",c.points);
    }else{
        printf("\nCustomer Not Found!\n");
    }
}

void viewPreviousPurchases(int customerId){
    char path[100];
    getShopFilePath("purchase.txt", path);
    FILE *fp = fopen(path, "r");
    int found = 0;

    if(fp == NULL){
        printf("\nNo Purchase Records Found!\n");
        return;
    }

    struct Purchase *purchases = NULL;
    int count = 0;
    int capacity = 16;
    purchases = (struct Purchase *)malloc(capacity * sizeof(struct Purchase));
    if (purchases == NULL) {
        printf("\nMemory Allocation Error!\n");
        fclose(fp);
        return;
    }

    struct Purchase p;
    while(fscanf(fp, "%d %d %s %s %f %f %f %f %f %s", 
                 &p.id, &p.customerId, p.customerName, p.phone, 
                 &p.subtotal, &p.discount, &p.tax, &p.total, &p.profit, p.date) == 10){
        if (p.customerId == customerId) {
            if (count >= capacity) {
                capacity *= 2;
                struct Purchase *temp = (struct Purchase *)realloc(purchases, capacity * sizeof(struct Purchase));
                if (temp == NULL) {
                    break;
                }
                purchases = temp;
            }
            purchases[count++] = p;
            found = 1;
        }
    }
    fclose(fp);

    if(!found || count == 0){
        printf("\nNo Previous Purchases Found!\n");
        free(purchases);
        return;
    }

    // Reverse the array so that the latest purchase is shown first
    for(int i = 0; i < count / 2; i++){
        struct Purchase temp = purchases[i];
        purchases[i] = purchases[count - 1 - i];
        purchases[count - 1 - i] = temp;
    }

    int page = 1, totalPages, start, end, i, op;
    totalPages = (count + 10 - 1) / 10;

    do{
        start = (page - 1) * 10;
        end = start + 10;
        if(end > count){
            end = count;
        }

        printf("\n=== Previous Purchases | Page %d of %d ===\n", page, totalPages);
        printf("%-6s %-12s %-12s %-10s %-12s %-12s\n", "ID", "Subtotal", "Discount", "Tax", "Total", "Date");
        printf("----------------------------------------------------------------------\n");
        for(i = start; i < end; i++){
            struct Purchase item = purchases[i];
            printf("%-6d %-12.2f %-12.2f %-10.2f %-12.2f %-12s\n", 
                   item.id, item.subtotal, item.discount, item.tax, item.total, item.date);
        }

        if(totalPages == 1){
            break;
        }

        printf("\n1. Next Page\n");
        printf("2. Previous Page\n");
        printf("3. Exit View\n");
        printf("Choice: ");
        if (!getIntegerInput(&op)) {
            op = 0;
        }

        if(op == 1 && page < totalPages){
            page++;
        }else if(op == 2 && page > 1){
            page--;
        }

    }while(op != 3);

    free(purchases);
}

void checkLoyaltyPoints(int customerId){
    struct Customer c;

    if(findCustomerById(customerId,&c)){
        printf("\nYour Loyalty Points: %d\n",c.points);
    }else{
        printf("\nCustomer Not Found!\n");
    }
}

void addLoyaltyPoints(int customerId, float total){
    struct Customer c;
    int points;

    if(customerId == 0){
        return;
    }

    if(findCustomerById(customerId,&c)){
        points = (int)(total / 100);
        c.points += points;
        updateCustomerRecord(c);
        printf("\nLoyalty Points Added: %d\n",points);
        printf("Current Points: %d\n",c.points);
    }
}

void syncMissingMedicines(){
    struct Medicine mList[MAX_MEDICINE];
    int mCount = readMedicines(mList);
    if(mCount == 0) return;

    char missPath[100];
    getShopFilePath("missing_medicine.txt", missPath);

    char missingList[MAX_MEDICINE][30];
    int missCount = 0, i;

    FILE *fmiss = fopen(missPath, "r");
    if(fmiss != NULL){
        while(fscanf(fmiss, "%s", missingList[missCount]) == 1){
            missCount++;
        }
        fclose(fmiss);
    }

    for (int idx = 0; idx < mCount; idx++) {
        struct Medicine m = mList[idx];
        if(m.qty == 0){
            int exists = 0;
            for(i = 0; i < missCount; i++){
                if(strcmp(missingList[i], m.name) == 0){
                    exists = 1;
                    break;
                }
            }
            if(!exists && missCount < MAX_MEDICINE){
                strcpy(missingList[missCount], m.name);
                missCount++;
            }
        }else if(m.qty > 0){
            for(i = 0; i < missCount; i++){
                if(strcmp(missingList[i], m.name) == 0){
                    int j;
                    for(j = i; j < missCount - 1; j++){
                        strcpy(missingList[j], missingList[j + 1]);
                    }
                    missCount--;
                    i--;
                }
            }
        }
    }

    fmiss = fopen(missPath, "w");
    if(fmiss != NULL){
        for(i = 0; i < missCount; i++){
            fprintf(fmiss, "%s\n", missingList[i]);
        }
        fclose(fmiss);
    }
}

void viewMissingMedicines(){
    syncMissingMedicines();
    char path[100];
    getShopFilePath("missing_medicine.txt", path);
    FILE *fp = fopen(path, "r");
    char name[30];
    int count = 1;

    if(fp == NULL){
        printf("\nNo Missing Medicines!\n");
        return;
    }

    printf("\n=== Missing Medicine List ===\n");
    while(fscanf(fp, "%s", name) == 1){
        printf("%d. %s\n", count, name);
        count++;
    }
    if(count == 1){
        printf("No Missing Medicines!\n");
    }
    fclose(fp);
}

void requestMissingMedicine(int customerId){
    if(customerId == 0){
        printf("\nOnly registered customers can request for a medicine!\n");
        return;
    }

    char name[30], path[100];
    printf("\nEnter Medicine Name to Request: ");
    scanf("%s", name);
    normalizeName(name);

    getShopFilePath("missing_medicine.txt", path);
    
    FILE *fp = fopen(path, "r");
    char existing[30];
    if(fp != NULL){
        while(fscanf(fp, "%s", existing) == 1){
            if(strcmp(existing, name) == 0){
                printf("\nThis medicine is already in the missing medicine list!\n");
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen(path, "a");
    if(fp == NULL){
        printf("\nUnable to process request!\n");
        return;
    }
    fprintf(fp, "%s\n", name);
    fclose(fp);

    printf("\nMedicine request submitted successfully! Added to missing list.\n");
}

void adminSalesDashboard(){
    char path[100];
    getShopFilePath("purchase.txt", path);

    FILE *fp = fopen(path, "r");
    struct Purchase p;
    float dailySales = 0.0, monthlySales = 0.0, totalSales = 0.0;
    float dailyProfit = 0.0, monthlyProfit = 0.0, totalProfit = 0.0;
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char todayStr[64], monthStr[64];
    sprintf(todayStr, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(monthStr, "%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);

    printf("\n========== Sales & Inventory Dashboard ==========\n");
    printf("Current Date: %s\n", todayStr);
    printf("-------------------------------------------------\n");

    if(fp != NULL){
        while(fscanf(fp, "%d %d %s %s %f %f %f %f %f %s", 
                     &p.id, &p.customerId, p.customerName, p.phone, 
                     &p.subtotal, &p.discount, &p.tax, &p.total, &p.profit, p.date) == 10){
            
            totalSales += p.total;
            totalProfit += p.profit;

            if(strncmp(p.date, todayStr, 10) == 0){
                dailySales += p.total;
                dailyProfit += p.profit;
            }
            if(strncmp(p.date, monthStr, 7) == 0){
                monthlySales += p.total;
                monthlyProfit += p.profit;
            }
        }
        fclose(fp);
    }

    printf("Daily Sales Record   : %.2f | Daily Profit: %.2f\n", dailySales, dailyProfit);
    printf("Monthly Sales Record : %.2f | Monthly Profit: %.2f\n", monthlySales, monthlyProfit);
    printf("Total All-Time Sales : %.2f | Total Profit: %.2f\n", totalSales, totalProfit);
    printf("-------------------------------------------------\n");

    struct Medicine mList[MAX_MEDICINE];
    int mCount = readMedicines(mList);
    int lowStockCount = 0;

    printf("\n=== Low Stock Alert (Qty <= 5) ===\n");
    printf("%-8s %-20s %-12s %-12s %-8s\n", "ID", "Name", "Company", "Category", "Qty");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < mCount; i++) {
        if(mList[i].qty <= 5){
            printf("%-8d %-20s %-12s %-12s %-8d\n", mList[i].id, mList[i].name, mList[i].company, mList[i].category, mList[i].qty);
            lowStockCount++;
        }
    }

    if(lowStockCount == 0){
        printf("All items have sufficient stock!\n");
    }
    printf("=================================================\n");
}

void customerDashboard(int customerId){
    int op;

    do{
        printf("\n--- Registered Customer Dashboard ---\n");
        printf("1. View Profile\n");
        printf("2. View Medicines\n");
        printf("3. Search Medicine\n");
        printf("4. Buy Medicine (Add to Cart)\n");
        printf("5. View Previous Purchases\n");
        printf("6. Check Loyalty Points\n");
        printf("7. Request Missing Medicine\n");
        printf("8. Logout\n");
        printf("Choice: ");
        if (!getIntegerInput(&op)) {
            op = 0;
        }

        if(op==1) viewCustomerProfile(customerId);
        else if(op==2) viewMedicine();
        else if(op==3) searchMedicine();
        else if(op==4) customerCheckout(customerId);
        else if(op==5) viewPreviousPurchases(customerId);
        else if(op==6) checkLoyaltyPoints(customerId);
        else if(op==7) requestMissingMedicine(customerId);

    }while(op!=8);
}

void guestDashboard(){
    int op;

    do{
        printf("\n--- Guest Dashboard ---\n");
        printf("1. View Medicines\n");
        printf("2. Search Medicine\n");
        printf("3. Buy Medicine (Add to Cart)\n");
        printf("4. Request Missing Medicine (Registered Only)\n");
        printf("5. Back\n");
        printf("Choice: ");
        if (!getIntegerInput(&op)) {
            op = 0;
        }

        if(op==1) viewMedicine();
        else if(op==2) searchMedicine();
        else if(op==3) customerCheckout(0);
        else if(op==4) requestMissingMedicine(0);

    }while(op!=5);
}

void addMedicine(){
    struct Medicine medList[MAX_MEDICINE];
    int count = readMedicines(medList);
    if(count >= MAX_MEDICINE){
        printf("\nInventory is Full!\n");
        return;
    }
    
    struct Medicine m;
    printf("\nID: ");
    if (!getIntegerInput(&m.id)) {
        printf("\nInvalid ID! Please enter a number.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (medList[i].id == m.id) {
            printf("\nError: Medicine with this ID already exists!\n");
            return;
        }
    }

    printf("Name: ");
    scanf("%s",m.name);
    normalizeName(m.name);
    printf("Price (Selling): ");
    if (scanf("%f",&m.price) != 1) {
        while (getchar() != '\n');
        printf("\nInvalid Price!\n");
        return;
    }
    printf("Quantity: ");
    if (!getIntegerInput(&m.qty)) {
        printf("\nInvalid Quantity!\n");
        return;
    }
    printf("Company: ");
    scanf("%s",m.company);
    normalizeName(m.company);
    printf("Category: ");
    scanf("%s",m.category);
    normalizeName(m.category);
    
    float purchasedPrice;
    printf("Purchased Price: ");
    if (scanf("%f",&purchasedPrice) != 1) {
        while (getchar() != '\n');
        printf("\nInvalid Price!\n");
        return;
    }
    
    char tempPriceStr[32];
    sprintf(tempPriceStr, "%.2f", purchasedPrice);
    computeMD5(tempPriceStr, m.purchasedPrice);

    medList[count] = m;
    writeMedicines(medList, count + 1);

    syncMissingMedicines();
    printf("\nMedicine Added Successfully!\n");
}

void viewMedicine(){
    struct Medicine m[MAX_MEDICINE];
    int count = readMedicines(m);
    int page=1, totalPages, start, end, i, op;

    if(count == 0){
        printf("\nNo Medicine Records Found!\n");
        return;
    }

    totalPages = (count + PAGE_SIZE - 1) / PAGE_SIZE;

    do{
        start = (page - 1) * PAGE_SIZE;
        end = start + PAGE_SIZE;
        if(end > count){
            end = count;
        }

        printf("\n========== Medicine List | Page %d of %d ==========\n",page,totalPages);
        printf("%-8s %-20s %-15s %-15s %-10s %-8s\n", "ID", "Name", "Company", "Category", "Price", "Qty");
        printf("--------------------------------------------------------------------------------\n");
        for(i=start; i<end; i++){
            printf("%-8d %-20s %-15s %-15s %-10.2f %-8d\n", m[i].id, m[i].name, m[i].company, m[i].category, m[i].price, m[i].qty);
        }

        if(totalPages == 1){
            break;
        }

        printf("\n1. Next Page\n");
        printf("2. Previous Page\n");
        printf("3. Exit View\n");
        printf("Choice: ");
        if (!getIntegerInput(&op)) {
            op = 0;
        }

        if(op==1 && page < totalPages){
            page++;
        }else if(op==2 && page > 1){
            page--;
        }

    }while(op!=3);
}

void searchMedicine(){
    struct Medicine m[MAX_MEDICINE];
    int count = readMedicines(m);
    if(count == 0){
        printf("\nNo Medicine Records Found!\n");
        return;
    }
    char key[30];
    int foundIdx = -1;

    printf("\nEnter Medicine Name: ");
    scanf("%s",key);
    normalizeName(key);

    for (int i = 0; i < count; i++) {
        if (strcmp(m[i].name, key) == 0) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx != -1) {
        printf("\nFound: ID: %d | Name: %s | Company: %s | Category: %s | Price: %.2f | Qty: %d\n",
               m[foundIdx].id, m[foundIdx].name, m[foundIdx].company, m[foundIdx].category, m[foundIdx].price, m[foundIdx].qty);
        
        if (m[foundIdx].qty == 0) {
            printf("\nStatus: OUT OF STOCK (Not Available)\n");
            printf("Recommendations (Same category '%s' from other companies):\n", m[foundIdx].category);
            int recCount = 0;
            printf("%-8s %-20s %-15s %-10s %-8s\n", "ID", "Name", "Company", "Price", "Qty");
            printf("----------------------------------------------------------------------\n");
            for (int i = 0; i < count; i++) {
                if (strcmp(m[i].category, m[foundIdx].category) == 0 && strcmp(m[i].company, m[foundIdx].company) != 0 && m[i].qty > 0) {
                    printf("%-8d %-20s %-15s %-10.2f %-8d\n", m[i].id, m[i].name, m[i].company, m[i].price, m[i].qty);
                    recCount++;
                }
            }
            if (recCount == 0) {
                printf("No alternatives available in stock.\n");
            }
        }
    } else {
        printf("\nMedicine '%s' Not Found!\n", key);
        
        int partialFound = 0;
        for (int i = 0; i < count; i++) {
            if (strstr(m[i].name, key) != NULL || strstr(key, m[i].name) != NULL) {
                if (!partialFound) {
                    printf("\nDid you mean one of these?\n");
                    printf("%-8s %-20s %-15s %-15s %-10s %-8s\n", "ID", "Name", "Company", "Category", "Price", "Qty");
                    printf("--------------------------------------------------------------------------------\n");
                    partialFound = 1;
                }
                printf("%-8d %-20s %-15s %-15s %-10.2f %-8d\n", m[i].id, m[i].name, m[i].company, m[i].category, m[i].price, m[i].qty);
            }
        }

        if (!partialFound) {
            printf("\nWould you like to search by category? (1 for Yes / 0 for No): ");
            int catChoice;
            if (getIntegerInput(&catChoice) && catChoice == 1) {
                char catKey[30];
                printf("Enter Category Name: ");
                scanf("%s", catKey);
                normalizeName(catKey);
                
                int catFound = 0;
                printf("\nMedicines in category '%s':\n", catKey);
                printf("%-8s %-20s %-15s %-10s %-8s\n", "ID", "Name", "Company", "Price", "Qty");
                printf("----------------------------------------------------------------------\n");
                for (int i = 0; i < count; i++) {
                    if (strcmp(m[i].category, catKey) == 0) {
                        printf("%-8d %-20s %-15s %-10.2f %-8d\n", m[i].id, m[i].name, m[i].company, m[i].price, m[i].qty);
                        catFound = 1;
                    }
                }
                if (!catFound) {
                    printf("No medicines found in this category.\n");
                }
            }
        }
    }
}

void updateMedicine(){
    struct Medicine m[MAX_MEDICINE];
    int count = readMedicines(m);
    if(count == 0){
        printf("\nNo Medicine Records Found!\n");
        return;
    }
    int id, found=0;

    printf("\nEnter Medicine ID to Update: ");
    if (!getIntegerInput(&id)) {
        printf("\nInvalid ID! Please enter a number.\n");
        return;
    }

    for(int i=0; i<count; i++){
        if(m[i].id == id){
            found=1;
            printf("Enter New Name: ");
            scanf("%s",m[i].name);
            normalizeName(m[i].name);
            printf("Enter New Price (Selling): ");
            if (scanf("%f",&m[i].price) != 1) {
                while (getchar() != '\n');
                printf("\nInvalid Price!\n");
                return;
            }
            printf("Enter New Quantity: ");
            if (!getIntegerInput(&m[i].qty)) {
                printf("\nInvalid Quantity!\n");
                return;
            }
            printf("Enter New Company: ");
            scanf("%s",m[i].company);
            normalizeName(m[i].company);
            printf("Enter New Category: ");
            scanf("%s",m[i].category);
            normalizeName(m[i].category);
            
            float purchasedPrice;
            printf("Enter New Purchased Price: ");
            if (scanf("%f",&purchasedPrice) != 1) {
                while (getchar() != '\n');
                printf("\nInvalid Price!\n");
                return;
            }
            char tempPriceStr[32];
            sprintf(tempPriceStr, "%.2f", purchasedPrice);
            computeMD5(tempPriceStr, m[i].purchasedPrice);
            break;
        }
    }

    if(found){
        writeMedicines(m, count);
        syncMissingMedicines();
        printf("\nMedicine Updated Successfully!\n");
    }else{
        printf("\nMedicine ID Not Found!\n");
    }
}

void deleteMedicine(){
    struct Medicine m[MAX_MEDICINE];
    int count = readMedicines(m);
    if(count == 0){
        printf("\nNo Medicine Records Found!\n");
        return;
    }
    int id, found=0;

    printf("\nEnter Medicine ID to Delete: ");
    if (!getIntegerInput(&id)) {
        printf("\nInvalid ID! Please enter a number.\n");
        return;
    }

    int writeIdx = 0;
    for(int i=0; i<count; i++){
        if(m[i].id == id){
            found=1;
        }else{
            m[writeIdx++] = m[i];
        }
    }

    if(found){
        writeMedicines(m, writeIdx);
        syncMissingMedicines();
        printf("\nMedicine Deleted Successfully!\n");
    }else{
        printf("\nMedicine ID Not Found!\n");
    }
}

void customerCheckout(int customerId){
    struct CartItem cart[MAX_CART];
    struct Customer cust;
    int cartCount = 0;
    int choice;
    char custName[30] = "Guest", custPhone[15] = "N/A";

    if(customerId != 0 && findCustomerById(customerId,&cust)){
        strcpy(custName,cust.name);
        strcpy(custPhone,cust.phone);
    }

    struct Medicine mList[MAX_MEDICINE];
    int count = readMedicines(mList);

    do{
        if(count == 0){
            printf("\nNo Medicine Available!\n");
            return;
        }

        int medId, reqQty, found = 0;
        printf("\nEnter Medicine ID to Add to Cart: ");
        if (!getIntegerInput(&medId)) {
            printf("\nInvalid ID! Please enter a number.\n");
            continue;
        }

        for(int i=0; i<count; i++){
            if(mList[i].id == medId){
                found = 1;
                printf("Enter Quantity: ");
                if (!getIntegerInput(&reqQty) || reqQty <= 0) {
                    printf("\nInvalid Quantity! Please enter a positive number.\n");
                    break;
                }

                int cartIdx = -1;
                for (int k = 0; k < cartCount; k++) {
                    if (cart[k].id == medId) {
                        cartIdx = k;
                        break;
                    }
                }

                int currentCartQty = (cartIdx != -1) ? cart[cartIdx].qty : 0;

                if(reqQty + currentCartQty > mList[i].qty){
                    printf("\nInsufficient Stock! Available: %d (In Cart: %d)\n", mList[i].qty, currentCartQty);
                }else if(cartIdx == -1 && cartCount >= MAX_CART){
                    printf("\nCart Limit Full!\n");
                }else{
                    if (cartIdx != -1) {
                        cart[cartIdx].qty += reqQty;
                    } else {
                        cart[cartCount].id = mList[i].id;
                        strcpy(cart[cartCount].name, mList[i].name);
                        cart[cartCount].price = mList[i].price;
                        cart[cartCount].qty = reqQty;
                        cartCount++;
                    }
                    printf("\nItem Added to Cart!\n");
                }
                break;
            }
        }

        if(!found){
            printf("\nMedicine Not Found!\n");
        }

        printf("Do you want to add more items? (1 for Yes / 0 for No): ");
        if (!getIntegerInput(&choice)) {
            choice = 0;
        }

    }while(choice != 0);

    if(cartCount == 0){
        printf("\nCart is Empty!\n");
        return;
    }

    float subtotal = 0, tax = 0, discount = 0, total = 0;
    float totalCost = 0;

    printf("\n=== Virtual Cart ===\n");
    printf("Name\tPrice\tQty\tTotal\n");
    for(int i=0; i<cartCount; i++){
        float itemTotal = cart[i].price * cart[i].qty;
        subtotal += itemTotal;
        printf("%s\t%.2f\t%d\t%.2f\n", cart[i].name, cart[i].price, cart[i].qty, itemTotal);
        
        for (int j = 0; j < count; j++) {
            if (mList[j].id == cart[i].id) {
                float purPrice = decryptPrice(mList[j].purchasedPrice);
                totalCost += purPrice * cart[i].qty;
                break;
            }
        }
    }

    if(customerId != 0){
        discount = subtotal * 0.10;
    }
    tax = (subtotal - discount) * 0.05;
    total = subtotal - discount + tax;
    float profit = (subtotal - discount) - totalCost;

    printf("-------------------------------\n");
    printf("Subtotal: %.2f\n", subtotal);
    if(customerId != 0){
        printf("Registered Customer Discount (10%%): %.2f\n", discount);
    }
    printf("Tax (5%%): %.2f\n", tax);
    printf("Total Payable: %.2f\n", total);

    float payAmount;
    printf("\nEnter Payment Amount: ");
    if (scanf("%f", &payAmount) != 1) {
        while (getchar() != '\n');
        printf("\nInvalid Amount! Payment Failed.\n");
        return;
    }

    if(payAmount < total){
        printf("\nPayment Failed! Insufficient Amount.\n");
        return;
    }

    for(int i=0; i<cartCount; i++){
        for(int j=0; j<count; j++){
            if(mList[j].id == cart[i].id){
                mList[j].qty -= cart[i].qty;
                break;
            }
        }
    }
    writeMedicines(mList, count);
    syncMissingMedicines();

    char purPath[100];
    getShopFilePath("purchase.txt", purPath);
    FILE *fpur = fopen(purPath,"a");
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char todayStr[64];
    sprintf(todayStr, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    if(fpur != NULL){
        fprintf(fpur,"%d %d %s %s %.2f %.2f %.2f %.2f %.2f %s\n",
                getNextPurchaseId(),customerId,custName,custPhone,subtotal,discount,tax,total,profit,todayStr);
        fclose(fpur);
    }

    addLoyaltyPoints(customerId,total);

    printf("\n====================================\n");
    printf("        PURCHASE CONFIRMATION       \n");
    printf("====================================\n");
    printf("Customer: %s | Phone: %s\n", custName, custPhone);
    if(customerId != 0){
        printf("Customer ID: %d\n",customerId);
    }
    printf("------------------------------------\n");
    for(int i=0; i<cartCount; i++){
        printf("%s x %d = %.2f\n", cart[i].name, cart[i].qty, cart[i].price * cart[i].qty);
    }
    printf("------------------------------------\n");
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Tax (5%%): %.2f\n", tax);
    printf("Grand Total: %.2f\n", total);
    printf("Paid: %.2f | Change: %.2f\n", payAmount, payAmount - total);
    printf("====================================\n");
    printf("  Payment Successful! Thank You!   \n");
    printf("====================================\n");
}

int main(){

    int choice,op,subOp,customerId;
    char confirmPass[40];
    char correctPass[40];

    // Force shop login at start
    while(currentShopId == 0){
        printf("\n===== Welcome to Medical Store System =====\n");
        printf("Please log in to your shop admin account first.\n");
        printf("1. Shop Admin Login\n");
        printf("2. Register New Shop\n");
        printf("3. Exit System\n");
        printf("Choice: ");
        int startupChoice;
        if(scanf("%d", &startupChoice) != 1) {
            while(getchar() != '\n');
            continue;
        }
        if(startupChoice == 1){
            shopLogin();
        } else if(startupChoice == 2){
            registerShop();
        } else if(startupChoice == 3){
            printf("\nExiting System. Goodbye!\n");
            return 0;
        } else {
            printf("\nInvalid Choice!\n");
        }
    }

    while(1){
        // Ensure a shop session is active (if we did a System Logout, force login again)
        if(currentShopId == 0){
            while(currentShopId == 0){
                printf("\n===== Welcome to Medical Store System =====\n");
                printf("Please log in to your shop admin account first.\n");
                printf("1. Shop Admin Login\n");
                printf("2. Register New Shop\n");
                printf("3. Exit System\n");
                printf("Choice: ");
                int startupChoice;
                if(scanf("%d", &startupChoice) != 1) {
                    while(getchar() != '\n');
                    continue;
                }
                if(startupChoice == 1){
                    shopLogin();
                } else if(startupChoice == 2){
                    registerShop();
                } else if(startupChoice == 3){
                    printf("\nExiting System. Goodbye!\n");
                    return 0;
                } else {
                    printf("\nInvalid Choice!\n");
                }
            }
        }

        printf("\n===== Medical Store System =====\n");
        printf("1. Admin Portal\n");
        printf("2. Customer Portal\n");
        printf("3. Exit\n");
        printf("Choice: ");
        if(scanf("%d",&choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch(choice){

        case 1:
            if(shopLogin()){
                do{
                    printf("\n--- Shop Admin Menu ---\n");
                    printf("1. View Dashboard\n");
                    printf("2. Inventory Management\n");
                    printf("3. Log out\n");
                    printf("Choice: ");
                    if(scanf("%d",&op) != 1) {
                        while(getchar() != '\n');
                        op = 0;
                        continue;
                    }

                    if(op==1) {
                        adminSalesDashboard();
                    }
                    else if(op==2) {
                        int invOp;
                        do {
                            printf("\n--- Inventory Management ---\n");
                            printf("1. Add Medicine\n");
                            printf("2. View Medicine\n");
                            printf("3. Update Medicine\n");
                            printf("4. Delete Medicine\n");
                            printf("5. View Missing Medicine\n");
                            printf("6. Back\n");
                            printf("Choice: ");
                            if(scanf("%d",&invOp) != 1) {
                                while(getchar() != '\n');
                                invOp = 0;
                                continue;
                            }

                            if(invOp==1) addMedicine();
                            else if(invOp==2) viewMedicine();
                            else if(invOp==3) updateMedicine();
                            else if(invOp==4) deleteMedicine();
                            else if(invOp==5) viewMissingMedicines();

                        } while(invOp != 6);
                    }
                    else if(op==3) {
                        int logoutOp;
                        do {
                            printf("\n--- Logout Options ---\n");
                            printf("1. System Log out (Full Shop Logout)\n");
                            printf("2. Log out (Portal Logout)\n");
                            printf("3. Back\n");
                            printf("Choice: ");
                            if(scanf("%d",&logoutOp) != 1) {
                                while(getchar() != '\n');
                                logoutOp = 0;
                                continue;
                            }

                            if(logoutOp == 1) {
                                printf("\nEnter Shop Admin Password to Confirm System Logout: ");
                                char tempConfirmPass[40];
                                scanf("%s", tempConfirmPass);
                                computeMD5(tempConfirmPass, confirmPass);
                                if(getShopPassword(currentShopId, correctPass)) {
                                    if(strcmp(confirmPass, correctPass) == 0) {
                                        currentShopId = 0;
                                        currentShopFolder[0] = '\0';
                                        printf("\nSystem Logout Successful! Active shop session cleared.\n");
                                        op = 3; // Break outer Admin menu
                                        break;
                                    } else {
                                        printf("\nIncorrect Password! System Logout Cancelled.\n");
                                    }
                                } else {
                                    printf("\nError: Could not retrieve shop password!\n");
                                }
                            }
                            else if(logoutOp == 2) {
                                printf("\nLogging out of Admin Portal...\n");
                                op = 3; // Break outer Admin menu
                                break;
                            }
                        } while(logoutOp != 3);
                    }

                }while(op!=3);
            }
            break;

        case 2:
            do{
                printf("\n--- Customer Option ---\n");
                printf("1. Registered Customer\n");
                printf("2. Guest\n");
                printf("3. Back\n");
                printf("Choice: ");
                if(scanf("%d",&op) != 1) {
                    while(getchar() != '\n');
                    op = 0;
                    continue;
                }

                if(op == 1){
                    do{
                        printf("\n--- Registered Customer Menu ---\n");
                        printf("1. Signup\n");
                        printf("2. Login\n");
                        printf("3. Back\n");
                        printf("Choice: ");
                        if(scanf("%d",&subOp) != 1) {
                            while(getchar() != '\n');
                            subOp = 0;
                            continue;
                        }

                        if(subOp == 1){
                            registerCustomer();
                        }
                        else if(subOp == 2){
                            customerId = customerLogin();
                            if(customerId != 0){
                                customerDashboard(customerId);
                            }
                        }
                    }while(subOp != 3);
                }
                else if(op == 2){
                    guestDashboard();
                }

            }while(op != 3);
            break;

        case 3:
            return 0;

        default:
            printf("Invalid Choice");
        }
    }

}
