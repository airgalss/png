#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

using namespace std;

const int MAX = 100; //成组链接法的最大空闲块数
const int RANGE = 1000; //生成随机块号的范围
//链表长度在[LISTLEN_MIN,LISTLEN_MAX) 之间
const int LISTLEN_MIN = 3; 
const int LISTLEN_MAX = 6;
const int SHELL_CMD = 20;//该程序的shell里命令的最大长度

class Stack{
    public:
        Stack():next(NULL),free(1){}
        ~Stack(){}
        bool push(int);
        bool pop(int*);
        int free;
        Stack* next;
        int blockNum;
        int blocks[MAX];
};

bool Stack::push(int block){
    if (free >= MAX) return false;
    blocks[free] = block;
    free++;
    return true;
}

bool Stack::pop(int* save){
    if(free <= 0) return false;
    free--;
    if(free == 0) *save = blockNum;
    else *save = blocks[free];
    return true;
}

class SuperBlock{
    public:
        static int end;
        static int* arr;
        static int* getArr();
        SuperBlock(); //随机初始化一个带有空闲块号的链表
        ~SuperBlock();
        void fetch(int* ,int );
        void pull(int* , int, int*, int);
        void add(int*,int);
        void del(int*,int);
        void display();
    private:
        Stack* head;
};


int SuperBlock::end = RANGE - 1;
int* SuperBlock::arr = getArr();

int* SuperBlock::getArr(){
    int* arr = new int[RANGE];
    for(int i = 0; i < RANGE; i++){
        arr[i] = i + 1;
    }
    return arr;
}

void SuperBlock::add(int *req,int num){
    Stack* pre = head;
    fetch(req,num);//从随机数池取数
    for(int i = 0; i < num; i++){
        if(!head -> push(req[i])){
            head = new Stack();
            head -> next = pre;
            head -> blockNum = req[i];
            head -> blocks[0] = pre -> blockNum;
            pre = head;
        }
    }
}

void SuperBlock::del(int *del,int num){
    int list[LISTLEN_MAX];//收集被回收的组自身的块号
    int m = 0;
    Stack* pre = head;
    for(int i = 0; i < num; i++){
        if(!head -> pop(del + i)){
            list[m++] = head -> blockNum;
            delete head;
            head = pre -> next;
            pre = head;
            i--;
        }
    }
    pull(list,m,del,num);//将所有释放的块号重新放回随机数堆
}

SuperBlock::SuperBlock(){
    //初始化一个随机范围为[1,RANGE]的随机数组,不包括0是因为超级块号为0
    srand((int)time(NULL));
    //先初始化第一组，因为这一组的空闲块未满100个
    head = new Stack();
    head -> free = rand() % (MAX - 1) + 1; //超级块的free值为[1,99],后面的free值总为100
    head -> next = NULL;
    int temp,pos;
    //初始化第一组的空闲块号
    for(int i = 0; end > 0 && i < head->free; end--,i++){
        pos = rand() % (end + 1);
        (head -> blocks)[i] = arr[pos]; 
        arr[pos] = arr[end];
    }
    //初始化第一组本身的块号
    pos = rand() % (end + 1);
    head -> blockNum = arr[pos];
    arr[pos] = arr[end];
    end--;
    //初始化链表后面组的参数
    int listLen = rand() %  (LISTLEN_MAX - LISTLEN_MIN) + LISTLEN_MIN; 
    Stack *cur = NULL, *pre = head;
    for(int i = 0; i < listLen - 1; i++){
        cur = new Stack();
        cur -> free = MAX;
        pre -> next = cur;
        for(int j = 0; end > 0 && j < cur -> free; end--,j++){
            pos = rand() % (end + 1);
            (cur -> blocks)[j] = arr[pos]; 
            arr[pos] = arr[end];
        }
        //初始化后面组本身的块号
        cur -> blockNum = pre -> blocks[0];
        pre = cur;
    }

    //对最后一组进行修正
    Stack* last = head;
    for(int i = 0; i < listLen - 1; i++){
        last = last -> next;
    }
    last -> blocks[0] = 0;
    if (last != head) last -> free = (MAX - 1);
}

void SuperBlock::fetch(int* req,int num){
    srand((int)time(NULL));
    int pos;
    for(int i = 0; i < num; i++){
        pos = rand() % (end + 1);
        req[i] = arr[pos];
        arr[pos] = arr[end];
        end--;
    }
}

void SuperBlock::pull(int* arr1,int m,int* arr2,int n){
    for(int i = 0; i < m; i++){
        arr[++end] = arr1[i];
    }
    for(int i = 0; i < n; i++){
        arr[++end] = arr2[i];
    }
}

void SuperBlock::display(){
    printf("\033[33;48m组号\033[0m   \033[34;48m盘块号\033[0m   \033[35;48m空闲块数\033[0m   \033[36;48m空闲盘块栈\033[0m\n");   
    int j = 1;
    for(Stack* cur = head; cur != NULL; cur = cur -> next,j++){
        printf("\033[33;48m%-7d\033[0m\033[34;48m%-9d\033[0m\033[35;48m%-11d\033[0m", j, cur -> blockNum, cur -> free);
        printf("\033[34;48m%d\033[0m ",cur -> blocks[0]);
        for(int i = 1; i < cur -> free; i++){
            printf("\033[36;48m%d\033[0m ",cur -> blocks[i]);
        }
        printf("\n");
    }
}

SuperBlock::~SuperBlock(){
    Stack *cur = head,*pre = NULL;
    while(cur != NULL){
        pre = cur;
        cur = cur -> next;
        delete pre;
    }
}

class Shell{
    public:
        Shell();
        ~Shell(){};
        static void printList(int*,int);
        void process();
        void rand();
        void show();
        void add(int);
        void del(int);
        void help();
    private:
        SuperBlock* s;
};

Shell::Shell():s(NULL){
    printf("try to use 'h' to get more information.\n"); 
    process();
}

void Shell::printList(int* arr,int num){
    printf("[ ");
    for(int i = 0; i < num; i++){
        printf("%d ",arr[i]);
    }
    printf("]\n");
}

void Shell::process(){
    char str[SHELL_CMD];
    int flag;
    while(true){
        printf("\033[31;48mCMD>\033[0m ");
        memset(str,0,SHELL_CMD);
        fgets(str,SHELL_CMD,stdin);
        *(strchr(str,'\n')) = '\0';
        if(strcmp(str,"help") == 0 || strcmp(str,"h") == 0) help();
        else if(strcmp(str,"exit") == 0) break;
        else if(strcmp(str,"rand") == 0) rand();
        else if(strcmp(str,"show") == 0) show();
        else if(strncmp(str,"add",3) == 0){ 
            char temp[10];
            int num;
            sscanf(str,"%s %d",temp,&num);
            add(num);
        }
        else if(strncmp(str,"del",3) == 0){
            char temp[10];
            int num;
            sscanf(str,"%s %d",temp,&num);
            del(num);
        }
        else fprintf(stderr,"invalid option.\n");
    }
}

void Shell::rand(){
    if(s != NULL)   delete s;
    s = new SuperBlock();
}

void Shell::help(){
    printf("h/help        display this help\n");
    printf("exit          exit this shell\n");
    printf("rand          randomly initialise an idle listed list\n");
    printf("show          show the listed list\n");
    printf("add [n]       release n free blocks, which would be put back to random number pool\n");
    printf("del [n]       allocate n free blocks, which would be fetched from random number pool\n");
}

void Shell::show(){
    if(s == NULL) fprintf(stderr,"please use \"rand\" to init\n");
    else s -> display();
}

void Shell::add(int num){
    if(s == NULL) fprintf(stderr,"please use \"rand\" to init\n");
    else{
        int* arr = new int[num];
        s -> add(arr,num);
        printList(arr,num);
    }
}

void Shell::del(int num){
    if(s == NULL) fprintf(stderr,"please use \"rand\" to init\n");
    else{
        int* arr = new int[num];
        s -> del(arr,num);
        printList(arr,num);
    }
}

int main(){
    Shell s;
    return 0;
}
