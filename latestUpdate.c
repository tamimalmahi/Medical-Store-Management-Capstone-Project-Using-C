#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<time.h>

#define MAX_MEDICINE 250
#define MAX_CART 50
#define PAGE_SIZE 20

struct Medicine{
    int id;
    char name[30];
    float price;
    int qty;
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
    char date[15]; 
};

struct Shop{
    int id;
    char name[30];
    char username[20];
    char password[20];
    char folderPath[50];
};

int currentShopId = 0;
char currentShopFolder[50] = "";

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
    scanf("%s", s.password);

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

    if(fp == NULL){
        printf("\nNo Registered Shops Found! Please Register First.\n");
        return 0;
    }

    printf("\n--- Shop Admin Login ---\n");
    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);

    while(fscanf(fp, "%d %s %s %s %s", &s.id, s.name, s.username, s.password, s.folderPath) == 5){
        if(strcmp(user, s.username) == 0 && strcmp(pass, s.password) == 0){
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
    FILE *fp=fopen(path,"r");
    struct Purchase p;
    int maxId=0;

    if(fp == NULL){
        return 1;
    }

    while(fscanf(fp,"%d %d %s %s %f %f %f %f %s",&p.id,&p.customerId,p.customerName,p.phone,&p.subtotal,&p.discount,&p.tax,&p.total,p.date)==9 ||
          fscanf(fp,"%d %d %s %s %f %f %f %f",&p.id,&p.customerId,p.customerName,p.phone,&p.subtotal,&p.discount,&p.tax,&p.total)==8){
        if(p.id > maxId){
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
    FILE *fp=fopen(path,"a");
    struct Customer c;

    if(fp == NULL){
        printf("\nUnable to Open Customer File!\n");
        return;
    }

    c.id = getNextCustomerId();
    c.points = 0;

    printf("\nEnter Customer Name: ");
    scanf("%s",c.name);
    printf("Enter Phone Number: ");
    scanf("%s",c.phone);
    printf("Enter Location Name: ");
    scanf("%s",c.location);

    fprintf(fp,"%d %s %s %s %d\n",c.id,c.name,c.phone,c.location,c.points);
    fclose(fp);

    printf("\nSignup Successful!\n");
    printf("Your Customer ID is: %d\n",c.id);
    printf("Use this ID for future login.\n");
}

int customerLogin(){
    int customerId;
    struct Customer c;

    printf("\nEnter Customer ID: ");
    scanf("%d",&customerId);

    if(findCustomerById(customerId,&c)){
        printf("\nLogin Successful! Welcome %s\n",c.name);
        return customerId;
    }

    printf("\nInvalid Customer ID!\n");
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
    FILE *fp=fopen(path,"r");
    struct Purchase p;
    int found=0;

    if(fp == NULL){
        printf("\nNo Purchase Records Found!\n");
        return;
    }

    printf("\n=== Previous Purchases ===\n");
    printf("ID\tSubtotal\tDiscount\tTax\tTotal\t\tDate\n");
    while(fscanf(fp,"%d %d %s %s %f %f %f %f %s",&p.id,&p.customerId,p.customerName,p.phone,&p.subtotal,&p.discount,&p.tax,&p.total,p.date)==9 ||
          fscanf(fp,"%d %d %s %s %f %f %f %f",&p.id,&p.customerId,p.customerName,p.phone,&p.subtotal,&p.discount,&p.tax,&p.total)==8){
        if(p.customerId == customerId){
            printf("%d\t%.2f\t\t%.2f\t\t%.2f\t%.2f\t%s\n",p.id,p.subtotal,p.discount,p.tax,p.total, (strlen(p.date) > 0 ? p.date : "N/A"));
            found=1;
        }
    }

    if(!found){
        printf("\nNo Previous Purchases Found!\n");
    }

    fclose(fp);
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
    char medPath[100], missPath[100];
    getShopFilePath("medicine.txt", medPath);
    getShopFilePath("missing_medicine.txt", missPath);

    FILE *fmed = fopen(medPath, "r");
    if(fmed == NULL) return;

    struct Medicine m;
    char missingList[MAX_MEDICINE][30];
    int missCount = 0, i;

    FILE *fmiss = fopen(missPath, "r");
    if(fmiss != NULL){
        while(fscanf(fmiss, "%s", missingList[missCount]) == 1){
            missCount++;
        }
        fclose(fmiss);
    }

    while(fscanf(fmed, "%d %s %f %d", &m.id, m.name, &m.price, &m.qty) == 4){
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
    fclose(fmed);

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
    char path[100], medPath[100];
    getShopFilePath("purchase.txt", path);
    getShopFilePath("medicine.txt", medPath);

    FILE *fp = fopen(path, "r");
    struct Purchase p;
    float dailySales = 0.0, monthlySales = 0.0, totalSales = 0.0;
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char todayStr[12], monthStr[8];
    sprintf(todayStr, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(monthStr, "%04d-%02d", tm.tm_year + 1900, tm.tm_mon + 1);

    printf("\n========== Sales & Inventory Dashboard ==========\n");
    printf("Current Date: %s\n", todayStr);
    printf("-------------------------------------------------\n");

    if(fp != NULL){
        while(fscanf(fp, "%d %d %s %s %f %f %f %f %s", &p.id, &p.customerId, p.customerName, p.phone, &p.subtotal, &p.discount, &p.tax, &p.total, p.date) == 9 ||
              fscanf(fp, "%d %d %s %s %f %f %f %f", &p.id, &p.customerId, p.customerName, p.phone, &p.subtotal, &p.discount, &p.tax, &p.total) == 8){
            totalSales += p.total;
            if(strlen(p.date) > 0){
                if(strncmp(p.date, todayStr, 10) == 0){
                    dailySales += p.total;
                }
                if(strncmp(p.date, monthStr, 7) == 0){
                    monthlySales += p.total;
                }
            }
        }
        fclose(fp);
    }

    printf("Daily Sales Record  : %.2f\n", dailySales);
    printf("Monthly Sales Record: %.2f\n", monthlySales);
    printf("Total All-Time Sales: %.2f\n", totalSales);
    printf("-------------------------------------------------\n");

    FILE *fmed = fopen(medPath, "r");
    struct Medicine m;
    int lowStockCount = 0;

    printf("\n=== Low Stock Alert (Qty <= 5) ===\n");
    printf("%-8s %-20s %-8s\n", "ID", "Name", "Qty");
    printf("----------------------------------\n");

    if(fmed != NULL){
        while(fscanf(fmed, "%d %s %f %d", &m.id, m.name, &m.price, &m.qty) == 4){
            if(m.qty <= 5){
                printf("%-8d %-20s %-8d\n", m.id, m.name, m.qty);
                lowStockCount++;
            }
        }
        fclose(fmed);
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
        scanf("%d",&op);

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
        scanf("%d",&op);

        if(op==1) viewMedicine();
        else if(op==2) searchMedicine();
        else if(op==3) customerCheckout(0);
        else if(op==4) requestMissingMedicine(0);

    }while(op!=5);
}

void addMedicine(){
    char path[100];
    getShopFilePath("medicine.txt", path);
    FILE *fp=fopen(path,"a");
    struct Medicine m;

    printf("\nID: ");
    scanf("%d",&m.id);
    printf("Name: ");
    scanf("%s",m.name);
    normalizeName(m.name);
    printf("Price: ");
    scanf("%f",&m.price);
    printf("Quantity: ");
    scanf("%d",&m.qty);

    fprintf(fp,"%d %s %.2f %d\n",m.id,m.name,m.price,m.qty);
    fclose(fp);

    syncMissingMedicines();
    printf("\nMedicine Added Successfully!\n");
}

void viewMedicine(){
    char path[100];
    getShopFilePath("medicine.txt", path);
    FILE *fp=fopen(path,"r");
    struct Medicine m[MAX_MEDICINE];
    int count=0, page=1, totalPages, start, end, i, op;

    if(fp == NULL){
        printf("\nNo Medicine Records Found!\n");
        return;
    }

    while(count < MAX_MEDICINE && fscanf(fp,"%d %s %f %d",&m[count].id,m[count].name,&m[count].price,&m[count].qty)==4){
        count++;
    }
    fclose(fp);

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
        printf("%-8s %-20s %-10s %-8s\n", "ID", "Name", "Price", "Qty");
        printf("-------------------------------------------------\n");
        for(i=start; i<end; i++){
            printf("%-8d %-20s %-10.2f %-8d\n", m[i].id, m[i].name, m[i].price, m[i].qty);
        }

        if(totalPages == 1){
            break;
        }

        printf("\n1. Next Page\n");
        printf("2. Previous Page\n");
        printf("3. Exit View\n");
        printf("Choice: ");
        scanf("%d",&op);

        if(op==1 && page < totalPages){
            page++;
        }else if(op==2 && page > 1){
            page--;
        }

    }while(op!=3);
}

void searchMedicine(){
    char path[100];
    getShopFilePath("medicine.txt", path);
    FILE *fp=fopen(path,"r");
    if(fp == NULL){
        printf("\nNo Medicine Records Found!\n");
        return;
    }
    struct Medicine m;
    char key[30];
    int found=0;

    printf("\nEnter Medicine Name: ");
    scanf("%s",key);
    normalizeName(key);

    while(fscanf(fp,"%d %s %f %d",&m.id,m.name,&m.price,&m.qty)==4){
        if(strcmp(key,m.name)==0){
            printf("\nFound: %d %s %.2f %d\n",m.id,m.name,m.price,m.qty);
            found=1;
        }
    }

    if(!found)
        printf("\nMedicine Not Found!\n");

    fclose(fp);
}

void updateMedicine(){
    char path[100], tempPath[100];
    getShopFilePath("medicine.txt", path);
    getShopFilePath("temp.txt", tempPath);

    FILE *fp=fopen(path,"r");
    FILE *temp=fopen(tempPath,"w");
    if(fp == NULL || temp == NULL){
        printf("\nNo Medicine Records Found!\n");
        if(fp != NULL) fclose(fp);
        if(temp != NULL) fclose(temp);
        return;
    }
    struct Medicine m;
    int id, found=0;

    printf("\nEnter Medicine ID to Update: ");
    scanf("%d",&id);

    while(fscanf(fp,"%d %s %f %d",&m.id,m.name,&m.price,&m.qty)==4){
        if(m.id == id){
            found=1;
            printf("Enter New Name: ");
            scanf("%s",m.name);
            normalizeName(m.name);
            printf("Enter New Price: ");
            scanf("%f",&m.price);
            printf("Enter New Quantity: ");
            scanf("%d",&m.qty);
        }
        fprintf(temp,"%d %s %.2f %d\n",m.id,m.name,m.price,m.qty);
    }

    fclose(fp);
    fclose(temp);

    remove(path);
    rename(tempPath,path);

    syncMissingMedicines();

    if(found)
        printf("\nMedicine Updated Successfully!\n");
    else
        printf("\nMedicine ID Not Found!\n");
}

void deleteMedicine(){
    char path[100], tempPath[100];
    getShopFilePath("medicine.txt", path);
    getShopFilePath("temp.txt", tempPath);

    FILE *fp=fopen(path,"r");
    FILE *temp=fopen(tempPath,"w");
    if(fp == NULL || temp == NULL){
        printf("\nNo Medicine Records Found!\n");
        if(fp != NULL) fclose(fp);
        if(temp != NULL) fclose(temp);
        return;
    }
    struct Medicine m;
    int id, found=0;

    printf("\nEnter Medicine ID to Delete: ");
    scanf("%d",&id);

    while(fscanf(fp,"%d %s %f %d",&m.id,m.name,&m.price,&m.qty)==4){
        if(m.id == id) found=1;
        else fprintf(temp,"%d %s %.2f %d\n",m.id,m.name,m.price,m.qty);
    }

    fclose(fp);
    fclose(temp);

    remove(path);
    rename(tempPath,path);

    syncMissingMedicines();

    if(found)
        printf("\nMedicine Deleted Successfully!\n");
    else
        printf("\nMedicine ID Not Found!\n");
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

    do{
        char medPath[100];
        getShopFilePath("medicine.txt", medPath);
        FILE *fp=fopen(medPath,"r");
        if(fp == NULL){
            printf("\nNo Medicine Available!\n");
            return;
        }

        struct Medicine m[MAX_MEDICINE];
        int count = 0, medId, reqQty, found = 0;

        while(count < MAX_MEDICINE && fscanf(fp,"%d %s %f %d",&m[count].id,m[count].name,&m[count].price,&m[count].qty)==4){
            count++;
        }
        fclose(fp);

        printf("\nEnter Medicine ID to Add to Cart: ");
        scanf("%d",&medId);

        for(int i=0; i<count; i++){
            if(m[i].id == medId){
                found = 1;
                printf("Enter Quantity: ");
                scanf("%d",&reqQty);

                if(reqQty > m[i].qty){
                    printf("\nInsufficient Stock! Available: %d\n", m[i].qty);
                }else if(cartCount >= MAX_CART){
                    printf("\nCart Limit Full!\n");
                }else{
                    cart[cartCount].id = m[i].id;
                    strcpy(cart[cartCount].name, m[i].name);
                    cart[cartCount].price = m[i].price;
                    cart[cartCount].qty = reqQty;
                    cartCount++;
                    printf("\nItem Added to Cart!\n");
                }
                break;
            }
        }

        if(!found){
            printf("\nMedicine Not Found!\n");
        }

        printf("Do you want to add more items? (1 for Yes / 0 for No): ");
        scanf("%d",&choice);

    }while(choice != 0);

    if(cartCount == 0){
        printf("\nCart is Empty!\n");
        return;
    }

    float subtotal = 0, tax = 0, discount = 0, total = 0;
    printf("\n=== Virtual Cart ===\n");
    printf("Name\tPrice\tQty\tTotal\n");
    for(int i=0; i<cartCount; i++){
        float itemTotal = cart[i].price * cart[i].qty;
        subtotal += itemTotal;
        printf("%s\t%.2f\t%d\t%.2f\n", cart[i].name, cart[i].price, cart[i].qty, itemTotal);
    }

    if(customerId != 0){
        discount = subtotal * 0.10;
    }
    tax = (subtotal - discount) * 0.05;
    total = subtotal - discount + tax;

    printf("-------------------------------\n");
    printf("Subtotal: %.2f\n", subtotal);
    if(customerId != 0){
        printf("Registered Customer Discount (10%%): %.2f\n", discount);
    }
    printf("Tax (5%%): %.2f\n", tax);
    printf("Total Payable: %.2f\n", total);

    float payAmount;
    printf("\nEnter Payment Amount: ");
    scanf("%f",&payAmount);

    if(payAmount < total){
        printf("\nPayment Failed! Insufficient Amount.\n");
        return;
    }

    char medPath[100], tempPath[100];
    getShopFilePath("medicine.txt", medPath);
    getShopFilePath("temp.txt", tempPath);

    FILE *fp = fopen(medPath,"r");
    FILE *temp = fopen(tempPath,"w");
    struct Medicine m;

    if(fp == NULL || temp == NULL){
        if(fp != NULL) fclose(fp);
        if(temp != NULL) fclose(temp);
        return;
    }

    while(fscanf(fp,"%d %s %f %d",&m.id,m.name,&m.price,&m.qty)==4){
        for(int i=0; i<cartCount; i++){
            if(m.id == cart[i].id){
                m.qty -= cart[i].qty;
            }
        }
        fprintf(temp,"%d %s %.2f %d\n",m.id,m.name,m.price,m.qty);
    }
    fclose(fp);
    fclose(temp);
    remove(medPath);
    rename(tempPath,medPath);

    syncMissingMedicines();

    char purPath[100];
    getShopFilePath("purchase.txt", purPath);
    FILE *fpur = fopen(purPath,"a");
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char todayStr[12];
    sprintf(todayStr, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    if(fpur != NULL){
        fprintf(fpur,"%d %d %s %s %.2f %.2f %.2f %.2f %s\n",getNextPurchaseId(),customerId,custName,custPhone,subtotal,discount,tax,total,todayStr);
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
    char confirmPass[20];
    char correctPass[20];

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
                                scanf("%s", confirmPass);
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
