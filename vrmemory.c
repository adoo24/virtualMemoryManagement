//
//  main.c
//  cPrac
//
//  Created by 김준서 on 2021/11/01.
//
/*
#include <stdio.h>
#include <stdlib.h>
int num=0;
int main(int argc,char *argv[]) {
    FILE *fp,*fs;
    unsigned addr,temp;
    char rw;
    int i,n=1;
    int inproceed;
    fp = fopen("gcc9.trace","r");
    fs = fopen("bzip10.trace","r");
    if(fp == NULL){
        printf("오픈 실패\n");
    }
    else printf("파일열기 성공\n");
    for(i=0;i<20;i++){
        n=n*2;
    }
    int arr[n];
    for(i=0;i<n;i++){
        arr[i]=i;
    }
    printf("%d is n  \n",n);
    while(1){
        inproceed=fscanf(fp,"%x %c",&addr,&rw);
        if(inproceed==-1) break;
        printf("%x ",addr);
        temp=addr>>12<<12;
        printf("%x\n",temp);
        inproceed=fscanf(fs,"%x %c",&addr,&rw);
        if(inproceed==-1) break;
        printf("%x ",addr);
        temp=addr>>12<<12;
        printf("%x\n",temp);
        printf("%d      %d(int)\n",arr[temp>>12],(int)temp);

    }
}
*/
//
// Virual Memory Simulator Homework
// One-level page table system with FIFO and LRU
// Two-level page table system with LRU
// Inverted page table with a hashing system
// Submission Year:
// Student Name:
// Student Number:
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define PAGESIZEBITS 12            // page size = 4Kbytes
#define VIRTUALADDRBITS 32        // virtual address space size = 4Gbytes

struct procEntry {
    char *traceName;            // the memory trace name
    int pid;                    // process (trace) id
    int ntraces;                // the number of memory traces
    int num2ndLevelPageTable;    // The 2nd level page created(allocated);
    int numIHTConflictAccess;     // The number of Inverted Hash Table Conflict Accesses
    int numIHTNULLAccess;        // The number of Empty Inverted Hash Table Accesses
    int numIHTNonNULLAcess;        // The number of Non Empty Inverted Hash Table Accesses
    int numPageFault;            // The number of page faults
    int numPageHit;                // The number of page hits
    struct pageTableEntry *firstLevelPageTable;
    FILE *tracefp;
}*procTable;
struct pageTableEntry{
    struct mainMemory *adr;
    unsigned addr;
    int procID;
    struct pageTableEntry *secondLevelPageTable;
    int valid;
};
struct mainMemory{
    struct pageTableEntry *point;
    int vacent;
}*MM;

struct LL{
    struct pageTableEntry *point;
    struct LL *next;
}*Called;
struct LL *Called = NULL;

struct hashTable{
    struct pageTableEntry *point;
    struct mainMemory *adr;
    int procNum;
    struct hashTable *next;
}*HT;
void hashEnqueue(int hash,struct pageTableEntry *page,int nFrame,int procNum){
    struct hashTable *t=(struct hashTable *)malloc(sizeof(struct hashTable));
    t->point=page;
    t->procNum=procNum;
    hash=hash%nFrame;
    if(HT[hash].next==NULL){
        HT[hash].next=t;
        t->next=NULL;
    }
    else{
        t->next=HT[hash].next;
        HT[hash].next=t;
    }
}
int isThere(int hash, struct pageTableEntry *page, int nFrame, int procNum){
    int num=0;
    hash=hash%nFrame;
    struct hashTable *p=HT[hash].next;
    if(p==NULL) {
        procTable[procNum].numIHTNULLAccess++;
        return -1;
    }
    if(p->point->addr==page->addr&&p->point->procID==procNum){
        procTable[procNum].numIHTNonNULLAcess++;
        procTable[procNum].numIHTConflictAccess++;
        return 0;
    }
    while(p->next!=NULL){
        if(p->point->addr==page->addr&&p->point->procID==procNum){
            procTable[procNum].numIHTConflictAccess=procTable[procNum].numIHTConflictAccess+num+1;
            procTable[procNum].numIHTNonNULLAcess++;
            return num;
        }
        p=p->next;
        num++;
        
    }
    if(p->point->addr==page->addr&&p->point->procID==procNum){
        procTable[procNum].numIHTNonNULLAcess++;
        procTable[procNum].numIHTConflictAccess+=num+1;
        return num;
    }
    else{
        procTable[procNum].numIHTNonNULLAcess++;
        procTable[procNum].numIHTConflictAccess+=num+1;
        return -1;
    }
}
int isThere1(int hash, struct pageTableEntry *page, int nFrame, int procNum){
    int num=0;
    hash=hash%nFrame;
    struct hashTable *p=HT[hash].next;
    if(p==NULL) {
        return -1;
    }
    if(p->point->addr==page->addr&&p->point->procID==procNum){
        return 0;
    }
    while(p->next!=NULL){
        if(p->point->addr==page->addr&&p->point->procID==procNum){
            return num;
        }
        p=p->next;
        num++;
        
    }
    if(p->point->addr==page->addr&&p->point->procID==procNum){
        return num;
    }
    else{
        return -1;
    }

}

struct pageTableEntry *hashDequeue(int hash, struct pageTableEntry* page, int nFrame, int procNum,int num){
    int i=0;
    hash=hash%nFrame;
    struct hashTable *p = HT[hash].next;
    struct hashTable *prev;
    if(num==0){
        HT[hash].next=p->next;
        struct pageTableEntry *t= p->point;
        free(p);
        return t;
    }
    for(i=0;i<num;i++){
        prev = p;
        p=p->next;
    }
    if(p->next==NULL){
        prev->next=NULL;
        struct pageTableEntry *t=p->point;
        free(p);
        return t;
    }
    else{
        prev->next=p->next;
        struct pageTableEntry *t=p->point;
        free(p);
        return t;
    }
    
}

void enqueue(struct pageTableEntry *page){
    if(page==NULL){printf("page가 널인데요?\n");}
    if(Called==NULL){
        struct LL *t=(struct LL *)malloc(sizeof(struct LL));
        t->point=page;
        t->next=NULL;
        Called=t;
        
    }
    else{
        struct LL *t=(struct LL *)malloc(sizeof(struct LL));
        t->point = page;
        t->next = Called;
        Called=t;
    }
}
struct pageTableEntry *lastqueue(){
    struct LL *t=Called;
    while(t->next!=NULL){
        t=t->next;
    }
    return t->point;
}
struct pageTableEntry *dequeue(struct pageTableEntry *r){
    
    if(r==NULL){
        struct LL *t=Called;
        struct LL *prev=Called;
        while(t->next!=NULL){
            prev=t;
            t=t->next;
        }
        if(t==Called){
            struct pageTableEntry *addr =t->point;
            Called=NULL;
            free(t);
            return addr;
        }
        prev->next=NULL;
        struct pageTableEntry *addr = t->point;
        free(t);
        return addr;
    }
    else{
        struct LL *q=Called;
        struct LL *u;
        if(q==NULL){
            return r;
        }
        if(q->point==r) {
            Called=q->next;
            struct pageTableEntry *add = q->point;
            free(q);
            return add;
        }
        while(q->next!=NULL){
            if(q->point==r){
                u->next=q->next;
                struct pageTableEntry *add = q->point;
                free(q);
                return add;
            }
            u=q;
            q=q->next;
        }
        if(q->point==r){
            u->next=NULL;
            struct pageTableEntry *add = q->point;
            free(q);
            return add;
        }
        return r;
    }
}
int isEmpty(int nFrame){ //MM이 하나라도 vacent 하면 그놈 반환 아니면 -1
    int i;
    for(i=0;i<nFrame;i++){
        if(MM[i].vacent==1) return i;
    }
    return -1;
}
int fifo(){
    static int p=-1;
    p++;
    return p;
}

void oneLevelVMSim(int numProcess,int nFrame,int simType) {
    int i,j,n=1,ok=1,empty;
    unsigned addr;
    char rw;
    for(i=0;i<20;i++){
        n=n*2;
    }
    for(i=0;i<numProcess;i++){
        procTable[i].firstLevelPageTable=(struct pageTableEntry *)malloc(sizeof(struct pageTableEntry)*n);
        for(j=0;j<n;j++){
            procTable[i].firstLevelPageTable[j].valid=0;
        }
    }
    j=0;
    while(ok!=-1){
        for(i=0;i<numProcess;i++){
            ok=fscanf(procTable[i].tracefp,"%x %c",&addr,&rw);
            if(ok==-1) break;
            procTable[i].ntraces++;
            addr=addr>>12;
            //printf("%x[%d] traceNUM %d",addr,j,procTable[i].ntraces);
            if(procTable[i].firstLevelPageTable[addr].valid==1){    //Page HIT
                procTable[i].numPageHit++;
                enqueue(dequeue(&procTable[i].firstLevelPageTable[addr]));
                //printf(" HIT\n");
            }
            else if(isEmpty(nFrame)!=-1){    //빈 MM발견
                empty = isEmpty(nFrame);
                procTable[i].firstLevelPageTable[addr].adr=&MM[empty];
                MM[empty].point=&procTable[i].firstLevelPageTable[addr];
                procTable[i].firstLevelPageTable[addr].valid=1;
                MM[empty].vacent=0;
                procTable[i].numPageFault++;
                enqueue(&procTable[i].firstLevelPageTable[addr]);
                //printf(" Fault(EMPTY MM)\n");
            }
            else if(simType==0){                //FIFO
                empty=fifo()%nFrame;
                MM[empty].point->valid=0;
                MM[empty].point=&procTable[i].firstLevelPageTable[addr];
                procTable[i].firstLevelPageTable[addr].valid=1;
                procTable[i].numPageFault++;
                //printf(" FAULT");
            }
            else if(simType==1){
                struct pageTableEntry *t=dequeue(NULL);
                enqueue(dequeue(&procTable[i].firstLevelPageTable[addr]));
                procTable[i].firstLevelPageTable[addr].adr=t->adr;
                t->adr=NULL;
                t->valid=0;
                procTable[i].firstLevelPageTable[addr].valid=1;
                procTable[i].numPageFault++;
                //printf(" FAULT(REPLACED)\n");
            }
            
                        
        }
        j++;
    }
    
    for(i=0; i < numProcess; i++) {
        printf("**** %s *****\n",procTable[i].traceName);
        printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
        printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
        printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
        assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
    }
    
}

void twoLevelVMSim(int numProcess,int nFrame,int simType,int secondLevel) {
    int i,j,k,n=1,n1=1,ok=1,empty;
    unsigned addr,addr1,addr2;
    char rw;
    for(i=0;i<secondLevel;i++){
        n=n*2;
    }
    for(i=0;i<(20-secondLevel);i++){
        n1=n1*2;
    }
    for(i=0;i<numProcess;i++){
        procTable[i].firstLevelPageTable=(struct pageTableEntry *)malloc(sizeof(struct pageTableEntry)*n);
        for(j=0;j<n;j++){
            procTable[i].firstLevelPageTable[j].secondLevelPageTable=(struct pageTableEntry *)malloc(sizeof(struct pageTableEntry)*(n1));
            for(k=0;k<n1;k++){
                procTable[i].firstLevelPageTable[j].secondLevelPageTable[k].valid=0;
            }
            procTable[i].firstLevelPageTable[j].valid=0;
            procTable[i].num2ndLevelPageTable=0;
        }
            }
    j=0;
    while(ok!=-1){
        for(i=0;i<numProcess;i++){
            ok=fscanf(procTable[i].tracefp,"%x %c",&addr1,&rw);
            if(ok==-1) break;
            procTable[i].ntraces++;
            addr1=addr1>>12;
            addr=addr1>>(20-secondLevel);
            addr2=addr1-(addr<<(20-secondLevel));
            //printf("%x",addr1);
            if(procTable[i].firstLevelPageTable[addr].valid==0){
                procTable[i].firstLevelPageTable[addr].valid=1;
                procTable[i].num2ndLevelPageTable++;
            }
            if(procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2].valid==1){    //Page HIT
                procTable[i].numPageHit++;
                enqueue(dequeue(&procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2]));
                //printf(" HIT\n");
            }
            else if(isEmpty(nFrame)!=-1){    //빈 MM발견
                empty = isEmpty(nFrame);
                procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2].adr=&MM[empty];
                MM[empty].point=&procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2];
                procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2].valid=1;
                MM[empty].vacent=0;
                procTable[i].numPageFault++;
                enqueue(&procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2]);
                //printf(" Fault(EMPTY MM)\n");
            }
            else if(simType==2){
                struct pageTableEntry *t=dequeue(NULL);
                enqueue(dequeue(&procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2]));
                procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2].adr=t->adr;
                t->adr=NULL;
                t->valid=0;
                procTable[i].firstLevelPageTable[addr].secondLevelPageTable[addr2].valid=1;
                procTable[i].numPageFault++;
                //printf(" FAULT(REPLACED)\n");
            }
            
                        
        }
        j++;
    }
    
    for(i=0; i < numProcess; i++) {
        printf("**** %s *****\n",procTable[i].traceName);
        printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
        printf("Proc %d Num of second level page tables allocated %d\n",i,procTable[i].num2ndLevelPageTable);
        printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
        printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
        assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
    }
}

void invertedPageVMSim(int numProcess,int nFrame,int simType) {
    int i,j,n=1,ok=1,num,hash,empty;
    unsigned addr;
    char rw;
    for(i=0;i<20;i++){
        n=n*2;
    }
    for(i=0;i<numProcess;i++){
        HT=(struct hashTable *)malloc(sizeof(struct hashTable)*nFrame);
        for(j=0;j<nFrame;j++){
            HT[j].next=NULL;
        }
        
    }
    j=0;
    while(ok!=-1){
        for(i=0;i<numProcess;i++){
            ok=fscanf(procTable[i].tracefp,"%x %c",&addr,&rw);
            if(ok==-1) break;
            procTable[i].ntraces++;
            addr=addr>>12;
            hash=addr+i;
            hash=hash%nFrame;
            //printf("%x[%d] traceNUM %d",addr,hash,procTable[i].ntraces);
            struct pageTableEntry *page = (struct pageTableEntry *)malloc(sizeof(struct pageTableEntry));
            page->addr=addr;
            page->procID=i;
            num=isThere(hash, page, nFrame, i);
            if(num!=-1){    //Page HIT
                struct hashTable *p= HT[hash].next;
                for(j=0;j<num;j++){
                    p=p->next;
                }
                enqueue(dequeue(p->point));
                procTable[i].numPageHit++;
                //printf(" HIT\n");
            }
            else if(isEmpty(nFrame)!=-1){    //빈 MM발견
                empty = isEmpty(nFrame);
                page->adr=&MM[empty];
                MM[empty].point=page;
                MM[empty].vacent=0;
                hashEnqueue(hash, page, nFrame, i);
                procTable[i].numPageFault++;
                enqueue(page);
                //printf(" Fault(EMPTY MM)\n");
            }
            else if(simType==3){
                struct pageTableEntry *t=dequeue(NULL);
                enqueue(dequeue(page));
                num=isThere1(t->addr+t->procID, t, nFrame, t->procID);
                if(num!=-1){
                    hashDequeue(t->addr+t->procID, t, nFrame, t->procID, num);
                }
                hashEnqueue(hash, page, nFrame, i);
                page->adr=t->adr;
                t->adr=NULL;
                procTable[i].numPageFault++;
                //printf(" FAULT(REPLACED)\n");
            }
            
                        
        }
        j++;
    }
    for(i=0; i < numProcess; i++) {
        printf("**** %s *****\n",procTable[i].traceName);
        printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
        printf("Proc %d Num of Inverted Hash Table Access Conflicts %d\n",i,procTable[i].numIHTConflictAccess);
        printf("Proc %d Num of Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNULLAccess);
        printf("Proc %d Num of Non-Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNonNULLAcess);
        printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
        printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
        assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
        assert(procTable[i].numIHTNULLAccess + procTable[i].numIHTNonNULLAcess == procTable[i].ntraces);
    }
}

int main(int argc, char *argv[]) {
    int i, simType,optind,numProcess,nFrame=1,phyMemSizeBits,secondLevel;
    // initialize procTable for Memory Simulations
    if(argv[1][1]=='s'){
        optind=1;
    }
    else optind=0;
    secondLevel=atoi(argv[optind+2]);
    simType=atoi(argv[1+optind]);
    numProcess=argc-optind-4;
    phyMemSizeBits=atoi(argv[3+optind]);
    for(i=0;i<phyMemSizeBits;i++){
        nFrame=nFrame*2;
    }
    nFrame=nFrame/4096;
    procTable=(struct procEntry *)malloc(sizeof(struct procEntry)*numProcess);
    for(i=0;i<numProcess;i++){
        procTable[i].traceName=argv[i+4+optind];
        procTable[i].pid=i;
        procTable[i].numPageHit=0;
        procTable[i].numPageFault=0;
        procTable[i].ntraces=0;
        procTable[i].numIHTNULLAccess=0;
        procTable[i].numIHTNonNULLAcess=0;
        procTable[i].numIHTConflictAccess=0;
    }
    MM=(struct mainMemory *)malloc(sizeof(struct mainMemory)*nFrame);
    for(i=0;i<nFrame;i++){
        MM[i].vacent=1;
    }
    for(i = 0; i < numProcess; i++) {
        
        // opening a tracefile for the process
        printf("process %d opening %s\n",i,argv[i + optind + 4]);
        procTable[i].tracefp = fopen(argv[i + optind + 4],"r");
        if (procTable[i].tracefp == NULL) {
            printf("ERROR: can't open %s file; exiting...",argv[i+optind+4]);
            exit(1);
        }
    }

    printf("Num of Frames %d Physical Memory Size %ld bytes\n",nFrame, (1L<<phyMemSizeBits));
    
    if (simType == 0) {
        printf("=============================================================\n");
        printf("The One-Level Page Table with FIFO Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        oneLevelVMSim(numProcess,nFrame,simType);
    }
    
    if (simType == 1) {
        printf("=============================================================\n");
        printf("The One-Level Page Table with LRU Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        oneLevelVMSim(numProcess,nFrame,simType);
    }
    
    if (simType == 2) {
        printf("=============================================================\n");
        printf("The Two-Level Page Table Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        twoLevelVMSim(numProcess,nFrame,simType,secondLevel);
    }
    
    if (simType == 3) {
        printf("=============================================================\n");
        printf("The Inverted Page Table Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        invertedPageVMSim(numProcess,nFrame,simType);
    }

    return(0);
}

