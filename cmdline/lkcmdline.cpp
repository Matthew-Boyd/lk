#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>

#include "../lk_absyn.cpp"
#include "../lk_env.cpp"
#include "../lk_eval.cpp"
#include "../lk_parse.cpp"
#include "../lk_lex.cpp"
#include "../lk_stdlib.cpp"
#include "../lk_invoke.cpp"
#include "../lk_math.cpp"

void fcall_out( lk::invoke_t &cxt )
{
	LK_DOC("out", "Output data to the console.", "(...):none");
	for (size_t i=0;i<cxt.arg_count();i++)
		std::cout << lk::to_utf8(cxt.arg(i).as_string());
		
	std::cout << std::flush;
}

void fcall_outln( lk::invoke_t &cxt )
{
	LK_DOC("outln", "Output data to the console followed by a newline.", "(...):none");
	for (size_t i=0;i<cxt.arg_count();i++)
		std::cout << lk::to_utf8(cxt.arg(i).as_string());
	
	std::cout << std::endl << std::flush;
}
void fcall_in(  lk::invoke_t &cxt )
{
#define NBUF 2048
	LK_DOC("in", "Input text from the user.", "(none):string");
	char buf[NBUF];
	fgets( buf, NBUF-1, stdin );
	cxt.result().assign( lk::from_utf8( buf ) );	
}

int main(int argc, char *argv[])
{
	FILE *fp_in = stdin;	
	bool parse_only = false;
	
	if ( argc > 2 
		&& strcmp( argv[2], "--parse" ) == 0 ) parse_only = true;
		
	if ( argc > 1 )
	{
		fp_in = fopen( argv[1], "r" );
		if (!fp_in)
		{
			printf("lk: could not read %s\n", argv[1]);
			return -1;
		}
	}
	
	lk::input_stream p( fp_in );
	lk::parser parse( p );

	lk::node_t *tree = parse.script();
	int code = 0;
	if ( parse.error_count() != 0 
		|| parse.token() != lk::lexer::END)
	{
		printf("parsing did not reach end of input\n");
	}
	else if ( !parse_only )
	{	
		lk::env_t env;
		env.register_func( fcall_in );
		env.register_func( fcall_out );
		env.register_func( fcall_outln );

		env.register_funcs( lk::stdlib_basic() );
		env.register_funcs( lk::stdlib_string() );
		env.register_funcs( lk::stdlib_math() );

		lk::vardata_t result;
		unsigned int ctl_id = lk::CTL_NONE;
		std::vector<lk_string> errors;
		if ( !lk::eval( tree, &env, errors, result, 0, ctl_id, 0, 0 ) )
		{
			code = -1;
			printf("eval failed\n");
			for (size_t i=0;i<errors.size();i++)
				printf( ">> %s\n", (const char*)errors[i].c_str() );
		}
	}
	
	for (int i=0; i<parse.error_count(); i++ )
		printf( ">> %s\n",  (const char*)parse.error(i).c_str() );

	delete tree;

	if ( fp_in != stdin )
		fclose(fp_in);
	
	return code;
}
