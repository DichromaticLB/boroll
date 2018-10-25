%{
#include<string>
#include<iostream>
#include"include/Bofiguration.hpp"
#include<stdio.h>

#define YYSTYPE std::string
#include"keyv.hpp"

int lines=1;
int baseIteration=0;
bool strsel=false;
%}

%option nounput
%x RULEMOD

%%

^#.*$ {}

\"[^"\n]*\"		  {yylval=std::string(yytext).substr(1,yyleng-2); strsel=true; return STR;}
[^ \t\r\n={},<>]+ {yylval=yytext; strsel=true; return STR;}

=[ \t\r]*\n {lines++; strsel=false; return '\n';}

=   {strsel=false; return '=';}

<RULEMOD,INITIAL>\n { lines++;}

<RULEMOD,INITIAL>[ \t\r] {}

\{ { if(strsel)BEGIN(RULEMOD); return '{'; }

<RULEMOD>{

\}						{ BEGIN(INITIAL);return '}';}
[0-9]{1,20}				{yylval=yytext; return NUM;}
[a-zA-Z][a-zA-Z0-9_.-]*	{yylval=yytext; return TOK;}
[^}] 					{return yytext[0];}


}


[^{] {return yytext[0];}



%%

void
yyerror (char const *s)
{
 	std::cerr<<"Line:"<<lines<<" "<<s<<std::endl;
}

void Bofiguration::setup()
{
	errno=0;
	yyin=fopen(Bofiguration::deff().c_str(),"r");
	if(yyin==NULL)
	{
		std::cerr<<strerror(errno)<<" File: "<<Bofiguration::deff()<<std::endl;
		exit(-1);
	}
	yyparse();
}

int yywrap(){
	if(baseIteration)
	{
		if(yyin!=NULL)
			fclose(yyin);
		return 1;
	}
	baseIteration=1;
	
	if(yyin!=NULL)
		fclose(yyin);
		
	Bofiguration::string s=Bofiguration::fetch<Bofiguration::string>("configFile");
	errno=0;
	yyin=fopen(s.c_str(),"r");
	if(yyin==NULL)
	{
		std::cerr<<strerror(errno)<<" File: "<<Bofiguration::fetch<Bofiguration::string>("configFile")<<std::endl;
		exit(-1);
	}
	
	return 0;
	}
