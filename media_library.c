/* Project 40: Media Library with Efficient Indexing & Search (C11) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HSIZE 211
#define PAGE 5
#define DATA_FILE "media_library.csv"

typedef struct Media {
    int id, year;
    char *title, *artist, *genre, *path;
    struct Media *next;
} Media;
typedef struct Index {
    Media *item;
    struct Index *next;
} Index;
typedef struct {
    Media *head;
    Index *title[HSIZE], *artist[HSIZE], *genre[HSIZE];
    int next_id;
} Library;

static char *dupstr(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (p) strcpy(p, s);
    return p;
}
static void trimnl(char *s) { s[strcspn(s, "\r\n")] = 0; }
static void prompt(const char *msg, char *buf, size_t n) {
    printf("%s", msg);
    if (!fgets(buf, (int)n, stdin)) buf[0] = 0;
    trimnl(buf);
}
static int number(const char *msg, int lo, int hi) {
    char b[80], *e; long v;
    for (;;) {
        prompt(msg, b, sizeof b);
        v = strtol(b, &e, 10);
        while (isspace((unsigned char)*e)) e++;
        if (e != b && !*e && v >= lo && v <= hi) return (int)v;
        puts("Invalid number. Try again.");
    }
}
static unsigned hash(const char *s) {
    unsigned long h = 5381;
    while (*s) h = h * 33u + (unsigned char)tolower((unsigned char)*s++);
    return (unsigned)(h % HSIZE);
}
static int eq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a++) != tolower((unsigned char)*b++)) return 0;
    }
    return !*a && !*b;
}
static int has(const char *s, const char *q) {
    size_t n = strlen(q);
    if (!n) return 1;
    for (; *s; s++) {
        size_t i = 0;
        while (i < n && s[i] && tolower((unsigned char)s[i]) == tolower((unsigned char)q[i])) i++;
        if (i == n) return 1;
    }
    return 0;
}
static void free_indexes(Library *l) {
    Index **tabs[] = {l->title, l->artist, l->genre};
    for (int t=0;t<3;t++) for (int i=0;i<HSIZE;i++) {
        Index *p=tabs[t][i];
        while(p){Index *n=p->next;free(p);p=n;}
        tabs[t][i]=NULL;
    }
}
static void add_index(Index **tab, const char *key, Media *m) {
    Index *p=malloc(sizeof *p);
    if(!p){perror("malloc");exit(EXIT_FAILURE);}
    p->item=m; unsigned h=hash(key); p->next=tab[h]; tab[h]=p;
}
static void rebuild(Library *l) {
    free_indexes(l);
    for(Media *m=l->head;m;m=m->next) {
        char *words=dupstr(m->title);
        if(!words){perror("malloc");exit(EXIT_FAILURE);}
        for(char *tok=strtok(words," \t-_,.;:!?()[]{}");tok;tok=strtok(NULL," \t-_,.;:!?()[]{}"))
            add_index(l->title,tok,m);
        free(words);
        add_index(l->artist,m->artist,m);
        add_index(l->genre,m->genre,m);
    }
}
static Media *make_media(int id,const char *t,const char *a,const char *g,int y,const char *p) {
    Media *m=calloc(1,sizeof *m);
    if(!m)return NULL;
    m->id=id;m->year=y;
    m->title=dupstr(t);m->artist=dupstr(a);m->genre=dupstr(g);m->path=dupstr(p);
    if(!m->title||!m->artist||!m->genre||!m->path){
        free(m->title);free(m->artist);free(m->genre);free(m->path);free(m);return NULL;
    }
    return m;
}
static Media *find_id(Library *l,int id) {
    for(Media *m=l->head;m;m=m->next)if(m->id==id)return m;
    return NULL;
}
static void insert(Library *l,Media *m) {
    m->next=l->head;l->head=m;
    if(m->id>=l->next_id)l->next_id=m->id+1;
    rebuild(l);
}
static void header(void) {
    puts("\nID  | TITLE                       | ARTIST/CREATOR       | GENRE          | YEAR | FILE PATH");
    puts("----+-----------------------------+----------------------+----------------+------+-------------------------");
}
static void show(Media *m) {
    printf("%-3d | %-27.27s | %-20.20s | %-14.14s | %-4d | %s\n",
        m->id,m->title,m->artist,m->genre,m->year,m->path);
}
static int count(Library *l){int n=0;for(Media*m=l->head;m;m=m->next)n++;return n;}
static void add_media(Library *l) {
    char t[512],a[512],g[256],p[1024];
    prompt("Title: ",t,sizeof t);prompt("Artist/Creator: ",a,sizeof a);
    prompt("Genre: ",g,sizeof g);int y=number("Release year (0-9999): ",0,9999);
    prompt("File path: ",p,sizeof p);
    if(!*t||!*a){puts("Title and artist are required.");return;}
    Media *m=make_media(l->next_id,t,a,g,y,p);
    if(!m){puts("Memory allocation failed.");return;}
    insert(l,m);printf("Added. Media ID: %d\n",m->id);
}
static void edit_media(Library *l) {
    int id=number("Media ID to edit: ",1,2147483647);Media*m=find_id(l,id);
    if(!m){puts("Record not found.");return;}
    char b[1024];puts("Press Enter to keep the current value.");
    printf("Title [%s]: ",m->title);if(fgets(b,sizeof b,stdin)){trimnl(b);if(*b){free(m->title);m->title=dupstr(b);}}
    printf("Artist [%s]: ",m->artist);if(fgets(b,sizeof b,stdin)){trimnl(b);if(*b){free(m->artist);m->artist=dupstr(b);}}
    printf("Genre [%s]: ",m->genre);if(fgets(b,sizeof b,stdin)){trimnl(b);if(*b){free(m->genre);m->genre=dupstr(b);}}
    printf("Year [%d] (blank keeps): ",m->year);if(fgets(b,sizeof b,stdin)){trimnl(b);if(*b){char*e;long v=strtol(b,&e,10);if(!*e&&v>=0&&v<=9999)m->year=(int)v;else puts("Invalid year; unchanged.");}}
    printf("File path [%s]: ",m->path);if(fgets(b,sizeof b,stdin)){trimnl(b);if(*b){free(m->path);m->path=dupstr(b);}}
    rebuild(l);puts("Updated.");
}
static void delete_media(Library *l) {
    int id=number("Media ID to delete: ",1,2147483647);Media **p=&l->head;
    while(*p&&(*p)->id!=id)p=&(*p)->next;
    if(!*p){puts("Record not found.");return;}
    Media*m=*p;*p=m->next;free(m->title);free(m->artist);free(m->genre);free(m->path);free(m);
    rebuild(l);puts("Deleted.");
}
static void display(Library*l) {
    if(!l->head){puts("Library is empty.");return;}
    header();for(Media*m=l->head;m;m=m->next)show(m);printf("\nTotal: %d\n",count(l));
}
static void search(Library*l,int field) {
    char q[256];prompt(field==0?"Title keyword: ":field==1?"Artist/creator: ":"Genre: ",q,sizeof q);
    if(!*q){puts("Enter a search term.");return;}
    Index **tab=field==0?l->title:field==1?l->artist:l->genre;
    int found=0;unsigned h=hash(q);
    for(Index*n=tab[h];n;n=n->next){
        Media*m=n->item;const char*k=field==0?m->title:field==1?m->artist:m->genre;
        if((field==0&&has(k,q))||eq(k,q)){if(!found)header();show(m);found++;}
    }
    if(!found)for(Media*m=l->head;m;m=m->next){
        const char*k=field==0?m->title:field==1?m->artist:m->genre;
        if(has(k,q)){if(!found)header();show(m);found++;}
    }
    if(!found)puts("No matching media found.");
}
static int cmptext(const char*a,const char*b) {
    while(*a&&*b){int x=tolower((unsigned char)*a),y=tolower((unsigned char)*b);if(x!=y)return x-y;a++;b++;}
    return (unsigned char)*a-(unsigned char)*b;
}
static int bytitle(const void*a,const void*b) {
    Media*x=*(Media**)a,*y=*(Media**)b;int c=cmptext(x->title,y->title);return c?c:x->id-y->id;
}
static int byyear(const void*a,const void*b) {
    Media*x=*(Media**)a,*y=*(Media**)b;return x->year!=y->year?x->year-y->year:cmptext(x->title,y->title);
}
static void sorted(Library*l) {
    int n=count(l);if(!n){puts("Library is empty.");return;}
    Media **v=malloc((size_t)n*sizeof *v);if(!v){puts("Out of memory.");return;}
    int i=0;for(Media*m=l->head;m;m=m->next)v[i++]=m;
    int c=number("Sort: 1.Title  2.Year: ",1,2);qsort(v,(size_t)n,sizeof *v,c==1?bytitle:byyear);
    int pages=(n+PAGE-1)/PAGE,page=0;
    for(;;){printf("\nPage %d/%d\n",page+1,pages);header();int end=(page+1)*PAGE;if(end>n)end=n;
        for(i=page*PAGE;i<end;i++)show(v[i]);if(pages==1)break;
        char b[20];prompt("N=next, P=previous, Q=quit: ",b,sizeof b);
        if(tolower((unsigned char)b[0])=='n'&&page+1<pages)page++;
        else if(tolower((unsigned char)b[0])=='p'&&page>0)page--;
        else if(tolower((unsigned char)b[0])=='q')break;
    }free(v);
}
static void csv_field(FILE*f,const char*s) {
    fputc('"',f);for(;*s;s++){if(*s=='"')fputc('"',f);fputc(*s,f);}fputc('"',f);
}
static int save_csv(Library*l,const char*name) {
    FILE*f=fopen(name,"w");if(!f){perror("CSV write");return 0;}
    fputs("id,title,artist,genre,year,path\n",f);
    for(Media*m=l->head;m;m=m->next){fprintf(f,"%d,",m->id);csv_field(f,m->title);fputc(',',f);
        csv_field(f,m->artist);fputc(',',f);csv_field(f,m->genre);fprintf(f,",%d,",m->year);csv_field(f,m->path);fputc('\n',f);}
    fclose(f);return 1;
}
static int parse_row(const char*line,char*fields[],int max) {
    int n=0;const char*p=line;
    while(*p&&n<max){size_t cap=strlen(p)+1,len=0;char*out=malloc(cap);if(!out)return -1;
        if(*p=='"'){p++;while(*p){if(*p=='"'&&p[1]=='"'){out[len++]='"';p+=2;}
            else if(*p=='"'){p++;break;}else out[len++]=*p++;}while(*p==' '||*p=='\t')p++;if(*p==',')p++;
        }else{while(*p&&*p!=',')out[len++]=*p++;if(*p==',')p++;}
        out[len]=0;fields[n++]=out;
    }return n;
}
static void import_csv(Library*l) {
    char name[512];prompt("CSV filename: ",name,sizeof name);FILE*f=fopen(name,"r");
    if(!f){perror("CSV read");return;}char line[8192];int added=0,skipped=0;
    if(!fgets(line,sizeof line,f)){puts("Empty CSV.");fclose(f);return;}
    while(fgets(line,sizeof line,f)){char*v[6]={0};int n=parse_row(line,v,6);
        if(n==6){char*e1,*e2;long id=strtol(v[0],&e1,10),y=strtol(v[4],&e2,10);
            if(!*e1&&!*e2&&id>0&&id<=2147483647&&y>=0&&y<=9999&&!find_id(l,(int)id)){
                Media*m=make_media((int)id,v[1],v[2],v[3],(int)y,v[5]);
                if(m){m->next=l->head;l->head=m;if(m->id>=l->next_id)l->next_id=m->id+1;added++;}else skipped++;
            }else skipped++;
        }else skipped++;
        for(int i=0;i<n;i++)free(v[i]);
    }fclose(f);rebuild(l);printf("Imported %d; skipped %d.\n",added,skipped);
}
static void load(Library*l) {
    FILE*f=fopen(DATA_FILE,"r");if(!f)return;char line[8192];if(!fgets(line,sizeof line,f)){fclose(f);return;}
    while(fgets(line,sizeof line,f)){char*v[6]={0};int n=parse_row(line,v,6);
        if(n==6){char*e1,*e2;long id=strtol(v[0],&e1,10),y=strtol(v[4],&e2,10);
            if(!*e1&&!*e2&&id>0&&id<=2147483647&&y>=0&&y<=9999&&!find_id(l,(int)id)){
                Media*m=make_media((int)id,v[1],v[2],v[3],(int)y,v[5]);
                if(m){m->next=l->head;l->head=m;if(m->id>=l->next_id)l->next_id=m->id+1;}
            }}for(int i=0;i<n;i++)free(v[i]);
    }fclose(f);rebuild(l);
}
static void cleanup(Library*l) {
    free_indexes(l);Media*m=l->head;while(m){Media*n=m->next;free(m->title);free(m->artist);free(m->genre);free(m->path);free(m);m=n;}
}
int main(void) {
    Library l={0};l.next_id=1;load(&l);
    puts("==============================================\n MEDIA LIBRARY - INDEXING & SEARCH\n==============================================");
    for(;;){puts("\n1.Add media\n2.Edit media\n3.Delete media\n4.Display all media\n5.Search title\n6.Search artist\n7.Search genre\n8.Sorted view + pagination\n9.Import CSV\n10.Save CSV\n0.Save and exit");
        int c=number("Choice: ",0,10);
        switch(c){case 1:add_media(&l);break;case 2:edit_media(&l);break;case 3:delete_media(&l);break;
        case 4:display(&l);break;case 5:search(&l,0);break;case 6:search(&l,1);break;case 7:search(&l,2);break;
        case 8:sorted(&l);break;case 9:import_csv(&l);break;case 10:if(save_csv(&l,DATA_FILE))puts("Saved.");break;
        case 0:if(save_csv(&l,DATA_FILE))puts("Saved.");cleanup(&l);puts("Goodbye.");return 0;}
    }
}