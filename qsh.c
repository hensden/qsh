/*
 * Author: Rohan Karnawat
 * Implementation of a shell in C
 * Quick SHell
 */

#include "header.h"
char **before_gt, **after_gt,**pre,**post;
bool quit_shell;
int bgf=0,found,found2,ap;
pid_t qsh_pid,qsh_pgid;
int pip,FD,arr[2],lastpip;
typedef struct jobs{
    char comm[1024];
    int exist;
}j;
j* hash[4194310];
j* comma[4194310];
int xx; char * LOL;
void create(int pid, char* arg)
{
    hash[pid]->exist = 1;
    strcpy(hash[pid]->comm,arg);
}


/* signal */
void new_func(int sig)
{
    int status;
    //pid_t var = wait3(&status,WNOHANG,(struct rusage*)NULL);
    pid_t var = waitpid(-1,&status,WNOHANG);
    if(var>0)
    {
	hash[var]->exist = 0;
	printf("process with %d pid exited successfully\n",var);
    }
}

/* Functions to obtain username@systemname:*/
char * get_u_name(void)
{
    char *buffer;
    buffer = (char*)malloc(BUFFER_SIZE*sizeof(char));
    buffer = getlogin();
    return buffer;
}
void get_s_name(void)
{
    struct utsname sys_name; 
    uname(&sys_name); 
    printf("%s",sys_name.nodename); 
}


void raiseError(char *str)
{
    fprintf(stderr,"%s",str);
}


char *get_command(void)
{
    size_t tot = 8192;
    /* Using getline(), so there is never shortage of space
     * getline() automatically/dynamically expands the block of 
     * memory for the string.
     * So, no need to realloc()
     */
    char *command = (char *)malloc(tot);
    int chars = getline(&command,&tot,stdin);
    int x = strlen(command);
    int i;
    for(i=x-1;i>=0;i++)
    {
	if(command[i]!=' ' && command[i]!='\t' && command[i]!='\n')
	    break;
	else
	    command[i]='\0';
    }
    return command;
}


char** tokenize(char * command,char * delimiter) // specify the delimiter // 
{

    int no_words = BUFFER_SIZE<<2,current_pointer = 0;
    char **words =(char**)malloc(no_words*sizeof(char*));

    if(!words){
	raiseError("ERROR");
    }

    char *token;
    token = strtok(command,delimiter);

    while(token)
    {	
	words[current_pointer++] = token;
	token = strtok(NULL,delimiter);      	/* deletes the present token after saving it */
    }

    //Terminate words
    words[current_pointer] = '\0';
    return words;
}

//Initialisation of Inbuilt Functions
void launch_pwd();void launch_cd(char **);void launch_echo(char **);void launch_exit(); /*void launch_pinfo(char **);*/


void launch(char ** args)
{
    //Function to call/perform bulit in commands/
    if(strcmp(args[0],"pwd")==0)
    {
	launch_pwd();
    }
    else if(strcmp(args[0],"cd")==0)
    {
	if(args[1]==NULL){
	    chdir(home_dir);
	}
	else
	    launch_cd(args);
    }
    else if(strcmp(args[0],"echo") == 0)
    {
	if(args[1]==NULL)
	    putchar('\n');
	else
	    launch_echo(args);
    }
    else if(!strcmp(args[0],"exit") || !strcmp(args[0],"quit"))
    {
	if(args[1])
	    raiseError("qsh: no arguments needed\n");
	launch_exit();
    }
    else if(strcmp(args[0],"jobs") == 0)
    {
	if(args[1])
	    raiseError("\"jobs\" requires only one argument");
	int i=0,j=1;
	for(i=1;i<4194310;i++)
	    if(hash[i]->exist==1){
		printf("%d: [%d] : [%s]\n",j,i,hash[i]->comm);j++;}
    }
    else if(strcmp(args[0],"kjob")==0)
    {
	if(strcmp(args[2],"9"))
	{
	    raiseError("qsh: Wrong Signal\n");
	    return;
	}
	// do something here //
	//args[1] = process index
	//args[2] = 9, for kill.
	int x = atoi(args[1]);int proc_id = -1;
	int i;int j=0;
	for(i=0;i<4194310;i++)
	{
	    if(hash[i]->exist==1)
	    {
		j++;
		if(j==x)
		{
		    proc_id = i;
		    break;
		}
	    }
	}
	//////////////////////////////////////////////////////////////////////
	if(proc_id == -1)
	{
	    raiseError("qsh: Bad indexing\n");
	}
	else
	{
	    hash[proc_id]->exist = 0;
	    kill(proc_id,9);
	}

    }
    else if(!strcmp(args[0],"overkill"))
    {
	if(args[1])
	    raiseError("qsh: \"overkill\" requires no arguments\n");
	int i;
	for(i=0;i<4194310;i++)
	{
	    if(hash[i]->exist==1)
	    {
		hash[i]->exist = 0;
		kill(i,9);
	    }
	}
    }
    else if(!strcmp(args[0],"fg"))
    {
	if(args[2])
	    raiseError("Only one argument is required");
	int v = atoi(args[1]);
	if(!v)
	{
	    raiseError("qsh: Bad indexing\n");
	    return;
	}
	int j = 0,i;
	for(i=0;i<4194310;i++)
	{
	    if(hash[i]->exist)
	    {
		j++;
		if(j==v)
		{
		    hash[i]->exist = 0;
		    break;
		}

	    }
	}
	if(i==4194310)
	{
	    raiseError("qsh: Bad indexing\n");
	    return;
	}
	printf("%s is now in foreground\n",hash[i]->comm);
	setpgid(i,qsh_pid);
	kill(i,SIGCONT);
	waitpid(i,NULL,WUNTRACED);
    }
}

void zhandler()
{
    pid_t ID;
    ID = xx;
    // Insert
    hash[ID]->exist = 1;
    strcpy(hash[ID]->comm,comma[ID]->comm);
    // Inserted
    kill(ID,SIGSTOP);
    putchar('\n');
}
void execute(char **args)
{
    if(args[0]==NULL)
	return;
    if(pip)
	pipe(arr);
    int ii,iter,i;bgf=0;
    before_gt = after_gt = pre = post = NULL;
    int argnum=0;
    for(argnum=0;args[argnum]!=NULL;argnum++);
    if(strcmp(args[argnum-1],"&")==0)
    {
	bgf=1;
	args[argnum-1]=NULL;
	free(args[argnum-1]);
    }
    //get present proc group
    pid_t pid = fork();
    if(pid && bgf)
    {
	create(pid,args[0]);
    }
    if(pid<0)
    {
	perror("qsh");
	_exit(-1);
    }
    else if(pid==0)
    {
	////////////////////////////////////////////////////////////////////////////////////
	if(pip)
	{
	    if(!lastpip)
	    {
		dup2(arr[1],STDOUT_FILENO);
	    }
	    dup2(FD,STDIN_FILENO);
	    close(arr[0]);
	}
	//////////////////////////////////////////////////////////////////////////////////
	for(ii=0;args[ii]!=NULL;ii++)
	{
	    if(!strcmp(args[ii],">"))
	    {
		//generate before_gt
		if(ii)
		{
		    before_gt = (char**)malloc(255*sizeof(char *));
		    for(iter=0;iter<ii;iter++)
			before_gt[iter] = strdup(args[iter]);
		}
		//generate after_gt 
		if(args[ii+1])
		{
		    after_gt = (char**)malloc(255*sizeof(char *));
		    for(iter=ii+1;iter<argnum;iter++)
			after_gt[iter-ii-1] = strdup(args[iter]);
		}
		if(before_gt && after_gt)
		    found = 1;
	    }
	    else
	    {
		int ite=0;
		for(;ite<strlen(args[ii]);ite++)
		    if(args[ii][ite] == '>')
		    {
			if(args[ii][ite+1]!='\0' && args[ii][ite+1]=='>')
			{
			    ite++;
			    ap=1;
			}
			int i;
			// generate before_gt
			if((ii==0 && ite) || ii)
			{
			    before_gt = (char**)malloc(255*sizeof(char *));
			    for(iter=0;iter<ii;iter++)
				before_gt[iter] = strdup(args[iter]);
			    before_gt[ii] = (char*)malloc(256*sizeof(char));
			    for(i=0;args[ii][i]!='>';i++)
				before_gt[ii][i] = args[ii][i];
			    before_gt[ii][i] = '\0';
			}
			//generate after_gt 
			i++;int i0=i;
			if((args[ii+1]==NULL && ite<strlen(args[ii])-1) || args[ii+1])
			{
			    after_gt = (char**)malloc(255*sizeof(char *));
			    after_gt[0] = (char *)malloc(256*sizeof(char));
			    for(;i<strlen(args[ii]);i++)
				after_gt[0][i-i0] = args[ii][i];
			    for(iter=ii+1;iter<argnum;iter++)
				after_gt[iter-ii-1] = strdup(args[iter]);}
			if(before_gt && after_gt)
			    found = 1;
		    }
	    }
	    if(found){
		break;}
	}
	if(!found)
	{
	    before_gt = (char**)malloc(255*sizeof(char *));
	    for(i=0;args[i];i++)
		before_gt[i] = strdup(args[i]);
	}
	// segment to detect "<" //
	for(ii=0;before_gt[ii]!=NULL;ii++);
	int argcnt = ii;
	for(ii=0;before_gt[ii]!=NULL;ii++)
	{
	    if(!strcmp(before_gt[ii],"<"))
	    {
		//generate pre
		if(ii)
		{
		    pre = (char**)malloc(255*sizeof(char *));
		    for(iter=0;iter<ii;iter++)
			pre[iter] = strdup(before_gt[iter]);
		}
		if(before_gt[ii+1])
		{
		    post = (char**)malloc(255*sizeof(char *));
		    for(iter=ii+1;iter<argcnt;iter++)
			post[iter-ii-1] = strdup(before_gt[iter]);
		}
		if(pre && post)
		    found2 = 1;

	    }
	    else{
		int ite=0;
		for(;ite<strlen(before_gt[ii]);ite++)
		    if(before_gt[ii][ite] == '<')
		    {
			int i;
			// generate pre
			if((ii==0 && ite) || ii)
			{
			    pre = (char**)malloc(255*sizeof(char *));
			    for(iter=0;iter<ii;iter++)
				pre[iter] = strdup(before_gt[iter]);
			    pre[ii] = (char*)malloc(256*sizeof(char));
			    for(i=0;before_gt[ii][i]!='<';i++)
				pre[ii][i] = before_gt[ii][i];
			    pre[ii][i] = '\0';
			}
			//generate post
			i++;int i0=i;
			if((before_gt[ii+1]==NULL && ite<strlen(before_gt[ii])-1) || before_gt[ii+1])
			{
			    post = (char**)malloc(255*sizeof(char *));
			    post[0] = (char *)malloc(256*sizeof(char));
			    for(;i<strlen(before_gt[ii]);i++)
			    {
				post[0][i-i0] = before_gt[ii][i];
			    }
			    for(iter=ii+1;iter<argcnt;iter++){
				if(post[0][0] == '\0')
				    post[iter-ii-1] = strdup(before_gt[iter]);
				else
				    post[iter-ii] = strdup(before_gt[iter]);

			    }
			}
			if(pre && post)
			    found2 = 1;
		    }
	    }
	    if(found2)
		break;
	}
	///////////////////////////////////////////////
	if(bgf==1)
	    setpgid(pid,getpid());
	if(!found && !found2)
	{
	    int j,launching=0;
	    for(j=0;command_list[j];j++)
	    {
		if(!strcmp(before_gt[0],command_list[j])){
		    launch(before_gt);launching = 1;}
	    }
	    if(launching)
		exit(1);
	    if(!launching && execvp(before_gt[0],before_gt)==-1){
		printf("Invalid Command\n");
		exit(1);
	    }
	}
	else if(found && !found2)
	{
	    if(after_gt[1]!=NULL)
		raiseError("-1Redirection requires only one file name\n");
	    int f;
	    if(!ap)
		f = open(after_gt[0],O_TRUNC|O_WRONLY|O_CREAT, 0666);
	    else
		f = open(after_gt[0],O_WRONLY|O_APPEND|O_CREAT, 0666);
	    dup2(f,1);
	    close(f);
	    int j,launching=0;
	    for(j=0;command_list[j];j++)
	    {
		if(!strcmp(before_gt[0],command_list[j])){
		    launch(before_gt);launching = 1;}
	    }
	    if(launching)
		exit(1);
	    if(!launching && execvp(before_gt[0],before_gt)==-1){
		printf("Invalid Command\n");
		exit(1);
	    }
	}
	else if(!found && found2)
	{
	    if(post[1])
		raiseError("0Redirection requires only one file name\n");
	    int fd = open(post[0],O_RDONLY,0666);
	    if(fd<0){
		perror("File descriptor");
		return;
	    }
	    if(dup2(fd,0)<0)
	    {
		perror("dup2");
		return;
	    }
	    close(fd);
	    if(execvp(pre[0],pre)==-1)
	    {
		raiseError("Invalid Command\n");exit(1);
	    }
	}
	else if(found && found2)
	{
	    if(post[1]){
		raiseError("1Redirection requires only one file name\n");
		return;
	    }
	    if(after_gt[1]!=NULL)
		raiseError("2Redirection requires only one file name\n");

	    int fd = open(post[0],O_RDONLY,0666);
	    if(fd<0){
		perror("File descriptor");
		return;
	    }
	    if(dup2(fd,0)<0)
	    {
		perror("dup2");
		return;
	    }
	    close(fd);
	    int f;
	    if(!ap)
		f = open(after_gt[0],O_TRUNC|O_WRONLY|O_CREAT, 0666);
	    else
		f = open(after_gt[0],O_WRONLY|O_APPEND|O_CREAT, 0666);
	    dup2(f,1);
	    close(f);
	    if(execvp(pre[0],pre)==-1)
	    {
		raiseError("Invalid Command\n");
		exit(1);
	    }
	}
	if(found)
	{
	    for(i=0;i<255;i++)
	    {
		if(before_gt[i])
		{
		    before_gt[i] = NULL;
		    free(before_gt[i]);
		}
		if(after_gt[i])
		{
		    after_gt[i] = NULL;
		    free(after_gt[i]);
		}
	    }
	    if(before_gt)
		before_gt = NULL;
	    if(after_gt)
		after_gt = NULL;
	    free(before_gt);
	    free(after_gt);
	}
	if(found2)
	{
	    for(i=0;i<255;i++)
	    {
		if(pre[i])
		{
		    pre[i]=NULL;free(pre[i]);
		}
		if(post[i])
		{	post[i]=NULL;free(post[i]);}
	    }
	    if(pre)
		pre=NULL;
	    if(post)
		post=NULL;
	    free(pre);free(post);
	}
	for(i=0;args[i];i++)
	    args[i] = NULL;
	free(args);
	return;
    }
    else
    {
	strcpy(comma[pid]->comm,args[0]);
	if(pip)
	{
	    close(arr[1]);
            FD = arr[0];
	}
	if(bgf==1)
	    tcsetpgrp(0,getpgrp());
	else
	{
	    	int status;
		xx = pid;
		pid_t wpid = waitpid(pid, &status, WUNTRACED);
	}
    }
}

void print_path(char *home)
{
    /*To generate command prompt*/
    printf(":");
    char absolute[BUFFER_SIZE];
    getcwd(absolute,sizeof(absolute));
    int home_len = strlen(home);
    int abs_len = strlen(absolute);

    /*If shell is running inside home directory or 
     * any of its' subdirectories, then print path 
     * relative to "~", if path is outside ~, then 
     * print absolute path */

    if(abs_len >= home_len)
    {
	int i;char temp[BUFFER_SIZE];
	for(i=0;i<home_len;i++)
	    temp[i] = absolute[i];
	temp[i] = '\0';

	if(strcmp(temp,home)==0)
	{
	    char new[BUFFER_SIZE];
	    new[0] = '~';
	    new[1] = '\0';

	    int j;
	    for(i=home_len,j=1;absolute[i]!='\0' && i<abs_len;i++,j++)
		new[j] = absolute[i];
	    new[j] = '\0';

	    strcpy(absolute,new);
	}
    }
    printf("%s>$ ",absolute);
}
void qsh_init();
void begin_qsh()
{
    qsh_init();
    signal(SIGTSTP,zhandler);
    quit_shell = false;
    char * u_name = get_u_name();
    getcwd(home_dir,sizeof(home_dir));
    char * command,**command2;
    char **command_line_args;
    char **command3;
    home_dir[strlen(home_dir)] = '\0';
    while(!quit_shell)
    {
	bgf=0;
	printf("<%s@",u_name);get_s_name();print_path(home_dir);
	int spaceflag=0,j;
	command = get_command();
	for(j=0;j<strlen(command);j++)
	{
	    if(command[j]!=' ' && command[j]!='\t' && command[j]!= '\n')
	    {
		spaceflag=1;
		break;
	    }
	}
	if(spaceflag==0)
	    continue;

	//printf("This is the last element: %c :",command[strlen(command) - 1]);
	command2 = tokenize(command,";");
	int jk;lastpip = 0;
	for(j=0;command2[j];j++)
	{
	    FD = 0;lastpip=0;pip = 0;
	    command3 = tokenize(command2[j],"|");//////////////////////////////////////////////////
	    if(command3[1]!=NULL)
	    {
		pip = 1;//////////////////////////////////////////////
	    }
	    found = 0;found2=0;ap=0;
	    before_gt = after_gt =NULL;
	    jk=0;for(;command3[jk]!=NULL;jk++)
	    {
		if(pip && command3[jk+1]==NULL)
		    lastpip = 1;
		before_gt = after_gt =NULL;
		found = 0;found2=0;ap=0;
		command_line_args = tokenize(command3[jk]," \n\t\r\a");
		if(!strcmp(command_line_args[0],"fg"))
		    launch(command_line_args);
		else if(!strcmp(command_line_args[0],"cd"))
		    launch(command_line_args);
		else
		    execute(command_line_args);
		int i;
		///////////i//////////////////////////////////////////////////////// 
		for(i=0;command_line_args[i];i++)
		    command_line_args[i] = NULL;
		free(command_line_args);
	    }
	    free(command3);///////////////////////////////////////////////////////////////////
	}

	/*Free all variables to avoid emory leak*/

	free(command2);
	free(command);
    }
    return;
}

int main(int argc, char** argv)
{
    /*Call main shell loop*/
    begin_qsh();
    return 0;
}

void qsh_init()
{
    // shell pid
    qsh_pid = getpid();
    qsh_pgid = getpgid(qsh_pid);
    // shell grpid

    // signals to be ignored
    signal(SIGINT,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);

    // Init bg process list
    int y=0;for(;y<4194310;y++)
    {
	hash[y] =(j *)malloc(sizeof(j));
	comma[y] =(j *)malloc(sizeof(j));
    }
    signal(SIGCHLD,new_func);
}

/* BUILTIN COMMANDS */
void launch_pwd()
{
    char buffer[BUFFER_SIZE];
    getcwd(buffer,sizeof(buffer));
    if(buffer == NULL)
	raiseError("Error in retrieving directory");
    else
	printf("%s\n",buffer);
    return;
}


void launch_cd(char ** directory)
{
    if(directory[2]!=NULL)
	raiseError("\"cd\" requires only one argument\n");
    else{
	if(!directory[1])
	    chdir(home_dir);
	else if(directory[1][0]=='~')
	{
	    if(directory[1][1] =='\0' || directory[1][1]=='\n')
		chdir(home_dir);
	}
	else if(chdir(directory[1])!=0)
	    raiseError("No such Directory\n");
    }
}


void launch_echo(char ** args)
{
    /* ECHO:
     * Implemented only -e and -n flags*/
    int nflag = 1,eflag = 0,Eflag = 0,i,j,k,l,argflag=1;
    for(i=1;args[i]!=NULL;i++)
    {
	l = strlen(args[i]);
	j=0;
	while(j<l)
	{
	    if(args[i][j] != '-' || argflag==0)
	    {
		argflag=0;
		if(args[i][j]=='"')
		    ++j;
		else if(args[i][j]!='\\')
		{
		    putchar(args[i][j++]);
		}
		else
		{
		    if (eflag==0){

			putchar(args[i][++j]);
			++j;
		    }
		    else if(eflag==1)
		    {
			if(args[i][j+1] == 'n'){
			    putchar('\n');
			    j+=2;}
			else if(args[i][j+1] == '\\'){
			    putchar('\\');
			    j+=2;}
			else if(args[i][j+1] == 't'){
			    putchar('\t');
			    j+=2;
			}
		    }
		}
	    }
	    else if(args[i][j] == '-' && argflag)
	    {
		if(!strcmp(args[i],"-n"))
		{
		    nflag = 0;
		    j+=2;
		}
		else if(!strcmp(args[i],"-ne"))
		{
		    nflag = 0;
		    eflag = 1;
		    j+=3;
		}
		else if(!strcmp(args[i],"-en"))
		{
		    nflag = 0;
		    eflag = 1;
		    j+=3;
		}
		else if(!strcmp(args[i],"-nE"))
		{
		    nflag = 0;
		    Eflag = 1;
		    j+=3;
		}
		else if(!strcmp(args[i],"-En"))
		{
		    nflag = 0;
		    Eflag = 1;
		    j+=3;
		}

		else if(!strcmp(args[i],"-e"))
		{
		    eflag = 1;
		    j+=2;
		}
		else if(!strcmp(args[i],"-eE"))
		{
		    eflag = 1;
		    Eflag = 1;
		    j+=3;
		}
		else if(!strcmp(args[i],"-Ee"))
		{
		    eflag = 1;
		    j+=3;
		    Eflag = 1;
		}
		else if(!strcmp(args[i],"-E"))
		{
		    Eflag = 1;
		    j+=2;
		}
	    }
	}putchar(' ');
    }
    if(nflag)
	putchar('\n');
    return;
}

void launch_exit(void)
{
    quit_shell = true;
    printf("qsh: ");
    kill(qsh_pid,9);


}
