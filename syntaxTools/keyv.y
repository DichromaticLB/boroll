%{
#include<string>
#include"include/Bofiguration.hpp"
#include"include/XMLPostFetch.hpp"
#define YYSTYPE std::string
void yyerror (char const *s);
int yylex();
static std::vector<std::string> _vecbuff;
static std::vector<XMLPostFilter::rule> rules;
XMLPostFilter::rule::SYM decode(const std::string v);
%}

%token STR NUM TOK

 
%right STR

%%

input:
	| input  kv 

kv:  STR '=' STR  { Bofiguration::put($1,$3); }
	|STR '\n'      { Bofiguration::put($1,""); }
	|STR '=' vec  { for(auto& str:_vecbuff)
							Bofiguration::putvec($1,str);
					if(_vecbuff.empty())
						Bofiguration::putvec($1);
					_vecbuff.clear();
					}
	| STR '{' filter  '}' {	auto& filter=XMLPostFilter::create($1); for(auto r:rules)filter.chain(r);	rules.clear();}
	
	
vec: '{' col '}'

col:
	| multi		    {_vecbuff.push_back($1);}
	| col ',' multi {_vecbuff.push_back($3);}

multi: STR     
	|multi STR	{$$=$1+" "+$2;}

filter: rule
	|   filter ',' rule
	
eq:  '=' '=' {$$="=";}
	|'!' '=' {$$="!";}
	
sym: '<'	
	|'>'	
	| eq	
		
		
rule: 	  TOK eq  TOK  mod { XMLPostFilter::rule r; r.key=$1;r.test=decode($2);r.val=$3;			r.required=$4=="!";r.isNum=false; rules.push_back(r);}
		| TOK sym NUM  mod { XMLPostFilter::rule r; r.key=$1;r.test=decode($2);r.num=std::stoul($3);r.required=$4=="!";r.isNum=true;  rules.push_back(r);}

mod:      {$$="";}
	| '!'

		
%%


XMLPostFilter::rule::SYM decode(const std::string v)
{
	if(v=="=")
		return XMLPostFilter::rule::SYM::EQ;
	else if(v=="!")
		return XMLPostFilter::rule::SYM::NOT_EQ;
	else if(v=="<")
		return XMLPostFilter::rule::SYM::LT;
	else
		return XMLPostFilter::rule::SYM::GT;
}