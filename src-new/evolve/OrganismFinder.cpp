//
// this class evaluates a find_expression for all organisms
// and sets their raddioactive tracer when the expression evaluates to true.
//

#include "stdafx.h"
#include "evolve.h"

#include "evolveDoc.h"
#include "evolveView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static KFORTH_OPERATIONS *FindOperations(void);

OrganismFinder::OrganismFinder(CString find_expression, bool reset_tracers)
{
	CString str;
	char errbuf[5000];

	m_reset_tracers = reset_tracers;

	if( find_expression.Find('{') == -1 ) {
		//
		// a proper kforth program must have the expression
		// surrounded by curly braces. If not specified, we
		// add them here.
		//
		str = "{ " + find_expression + " }";
	} else {
		str = find_expression;
	}

	m_kfp = kforth_compile(str, FindOperations(), errbuf);
	if( m_kfp == NULL ) {
		error		= true;
		error_message	= errbuf;
	} else {
		error		= false;
		error_message	= "";
	}
}

OrganismFinder::~OrganismFinder()
{
	if( m_kfp ) {
		kforth_delete(m_kfp);
	}
}

void OrganismFinder::execute(UNIVERSE *u)
{
	ORGANISM *o;
	KFORTH_MACHINE *kfm;
	KFORTH_INTEGER sum_energy, sum_age;
	int oenergy;

	ASSERT( u != NULL );
	ASSERT( !error );

	if( m_reset_tracers ) {
		Universe_ClearTracers(u);
	}

	//
	// Examine all organisms and compute
	// the min/max/avg values
	//
	sum_energy	= 0;
	sum_age		= 0;

	min_energy	= 999999;
	max_energy	= -1;

	min_age		= 999999;
	max_age		= -1;

	max_num_cells	= -1;

	for(o=u->organisms; o != NULL; o=o->next) {
		//
		// Find min/max/avg constants
		//
		oenergy=Organism_Nrg_Sum(o);
		sum_energy += oenergy;
		sum_age += o->age;

		if( oenergy < min_energy )
			min_energy = oenergy;

		if( oenergy > max_energy )
			max_energy = oenergy;

		if( o->age < min_age )
			min_age = o->age;

		if( o->age > max_age )
			max_age = o->age;

		if( o->ncells > max_num_cells )
			max_num_cells = o->ncells;

	}

	if( u->norganism > 0 ) {
		avg_energy	= (int)(sum_energy / u->norganism);
		avg_age		= (int)(sum_age / u->norganism);
	} else {
		avg_energy	= 0;
		avg_age		= 0;
	}

	kfm = kforth_machine_make(m_kfp, this);
	ASSERT( kfm != NULL );

	for(o=u->organisms; o != NULL; o=o->next) {
		if( evalute(kfm, o) ) {
			o->oflags |= ORGANISM_FLAG_RADIOACTIVE;
		}
	}

	kforth_machine_delete(kfm);
	m_kfp=NULL; MARK;

}

//
// execute KFORTH program. If top of stack is non-zero then
// return true, else return false.
//
bool OrganismFinder::evalute(KFORTH_MACHINE *kfm, ORGANISM *o)
{
	KFORTH_INTEGER val;
	int n;

	ASSERT( kfm != NULL );
	ASSERT( o != NULL );

	organism = o;
	kforth_machine_reset(kfm);

	//
	// Run machine until terminated, or number of
	// steps exceeds 1,000.
	//
	for(n=0; n < 1000; n++) {
		kforth_machine_execute(kfm);
		if( kfm->terminated ) {
			break;
		}
	}

	if( kfm->terminated ) { 
		if(kfm->num_s.pos==1){
			val = kforth_num_stack_pop(kfm);
			if( val == 0 ) {
				// non-match
				return false;
			}else{
				return true;
			}
		} else {
			//
			// expression terminated but data stack is
			// something other than 1 in length, we
			// will treat this case as false.
			//
			return false;
		}
	} else {
		//
		// somehow the damn expression never finished within
		// 1000 steps without terminated. Therefore we
		// just assume a bug and return false.
		// A non-looping, non-recursive
		// expression should always terminate.
		//
		return false;
	}
}

static void FindOpcode_ID(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->id);
}

static void FindOpcode_PARENT1(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->parent1);
}

static void FindOpcode_PARENT2(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->parent2);
}

static void FindOpcode_STRAIN(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	kforth_num_stack_push(kfm, o->strain+1);
}

static void FindOpcode_ENERGY(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm,Organism_Nrg_Sum(o));
}

static void FindOpcode_SECRETIN(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	UNIVERSE *u;
	UNIVERSE_GRID grid;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	u=o->universe;
	Grid_Get(u,o->cells->x,o->cells->y,&grid); MARK;
	kforth_num_stack_push(kfm, grid.secretin/s_); 
}

static void FindOpcode_GENERATION(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->generation);
}

static void FindOpcode_NUM_CELLS(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->ncells);
}

static void FindOpcode_AGE(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->age);
}

static void FindOpcode_DIRECTION(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kforth_num_stack_push(kfm, o->direction);
}

static void FindOpcode_NCHILDREN(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *organism, *o;
	UNIVERSE *u;
	KFORTH_INTEGER num_living_children;

	ofc = (OrganismFinder*) kfm->client_data;
	organism = ofc->organism;
	u = organism->universe;

	num_living_children = 0;
	for(o = u->organisms; o; o=o->next) {
		if( o->parent1 == organism->id || o->parent2 == organism->id ) {
			num_living_children += 1;
		}
	}

	kforth_num_stack_push(kfm, num_living_children);
}

static void FindOpcode_EXECUTING(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	CELL *c;
	KFORTH_INTEGER val;
	bool found;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;


	val = kforth_num_stack_pop(kfm);

	found = false;
	for(c=o->cells; c != NULL; c=c->next) {
		if( c->kfm->program->operand[c->kfm->cb][0] == val ) {
			found = true;
			break;
		}
	}

	if( found ) {
		kforth_num_stack_push(kfm, 1);
	} else {
		kforth_num_stack_push(kfm, 0);
	}

}

static void FindOpcode_SPORE(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	kforth_num_stack_push(kfm,o->cells->spore);  MARK;
}

static void FindOpcode_CALL_SIZE(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	kforth_num_stack_push(kfm,o->cells->kfm->call_stack_size);  MARK;
}

static void FindOpcode_IDENTIFIER(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	kforth_num_stack_push(kfm,o->cells->marker);  MARK;
}

static void FindOpcode_XY_ODD(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	CELL *c;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	c = o->cells;  MARK;

	kforth_num_stack_push(kfm,(c->x+c->y)%2); 
}

static void FindOpcode_NRG_WEIGH(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	int i,sum;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	sum=0;
	for(i=0;i<4;i++)
		sum+=o->cells->weigh_energy[i];

	kforth_num_stack_push(kfm,sum/4/ei);  MARK;
}

static void FindOpcode_SCT_WEIGH(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	int i,sum;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	sum=0;
	for(i=0;i<4;i++)
		sum+=o->cells->weigh_secretin[i];

	kforth_num_stack_push(kfm,sum/4/s_);  MARK;
}
static void FindOpcode_NUM_CB(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	kforth_num_stack_push(kfm,o->cells->kfm->program->nblocks);  MARK;
}

static void FindOpcode_NUM_INST(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	KFORTH_PROGRAM *kfp;
	int sum,cb;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;
	kfp=o->cells->kfm->program;

	sum=0;
	for(cb=0;cb<kfp->nblocks;cb++){
		sum+=kfp->block_len[cb];
	}

	kforth_num_stack_push(kfm,sum);  MARK;
}

static void FindOpcode_NUM_DEAD(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;
	ORGANISM *o;
	CELL *c;
	KFORTH_INTEGER num_dead;

	ofc = (OrganismFinder*) kfm->client_data;
	o = ofc->organism;

	num_dead = 0;
	for(c=o->cells; c != NULL; c=c->next) {
		if( c->kfm->terminated ) {
			num_dead += 1;
		}
	}

	kforth_num_stack_push(kfm, num_dead);
}

static void FindOpcode_MAX_ENERGY(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->max_energy);
}

static void FindOpcode_MIN_ENERGY(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->min_energy);
}

static void FindOpcode_AVG_ENERGY(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->avg_energy);
}

static void FindOpcode_MAX_AGE(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->max_age);
}

static void FindOpcode_MIN_AGE(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->min_age);
}

static void FindOpcode_AVG_AGE(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->avg_age);
}

static void FindOpcode_MAX_NUM_CELLS(KFORTH_MACHINE *kfm)
{
	OrganismFinder *ofc;

	ofc = (OrganismFinder*) kfm->client_data;
	kforth_num_stack_push(kfm, ofc->max_num_cells);
}


//
// A "once" function, always returns pointer to same static table
// for all instances and all callers.
//
// Returns a table of KFORTH operations for the FIND feature.
//
static KFORTH_OPERATIONS *FindOperations(void)
{
	static int first_time = 1;
	static KFORTH_OPERATIONS kfops_table[ KFORTH_OPS_LEN ];
	KFORTH_OPERATIONS *kfops;
	int i;

	if( first_time ) {
		first_time = 0;
		kfops = kforth_ops_make();

		kforth_ops_add(kfops,	"n",			kfop_insn);
		kforth_ops_add(kfops,	"ID",			FindOpcode_ID);
		kforth_ops_add(kfops,	"PARENT1",		FindOpcode_PARENT1);
		kforth_ops_add(kfops,	"PARENT2",		FindOpcode_PARENT2);
		kforth_ops_add(kfops,	"STRAIN",		FindOpcode_STRAIN);
		kforth_ops_add(kfops,	"ENERGY",		FindOpcode_ENERGY);
		kforth_ops_add(kfops,	"SECRETIN",		FindOpcode_SECRETIN);
		kforth_ops_add(kfops,	"GENERATION",		FindOpcode_GENERATION);
		kforth_ops_add(kfops,	"NUM-CELLS",		FindOpcode_NUM_CELLS);
		kforth_ops_add(kfops,	"AGE",			FindOpcode_AGE);
		kforth_ops_add(kfops,	"DIRECTION",			FindOpcode_DIRECTION);
		kforth_ops_add(kfops,	"NCHILDREN",		FindOpcode_NCHILDREN);
		kforth_ops_add(kfops,	"EXECUTING",		FindOpcode_EXECUTING);
		kforth_ops_add(kfops,	"SPORE",		FindOpcode_SPORE);
		kforth_ops_add(kfops,	"IDENTIFIER",		FindOpcode_IDENTIFIER);
		kforth_ops_add(kfops,	"XY-ODD",		FindOpcode_XY_ODD);
		kforth_ops_add(kfops,	"NRG-W",		FindOpcode_NRG_WEIGH);
		kforth_ops_add(kfops,	"SCT-W",		FindOpcode_SCT_WEIGH);
		kforth_ops_add(kfops,	"NUM-CB",		FindOpcode_NUM_CB);
		kforth_ops_add(kfops,	"NUM-INST",		FindOpcode_NUM_INST);
		kforth_ops_add(kfops,	"CALL-SIZE",		FindOpcode_CALL_SIZE);
		//kforth_ops_add(kfops,	"NUM-DEAD",		FindOpcode_NUM_DEAD);
		kforth_ops_add(kfops,	"MAX-ENERGY",		FindOpcode_MAX_ENERGY);
		kforth_ops_add(kfops,	"MIN-ENERGY",		FindOpcode_MIN_ENERGY);
		kforth_ops_add(kfops,	"AVG-ENERGY",		FindOpcode_AVG_ENERGY);
		kforth_ops_add(kfops,	"MAX-AGE",		FindOpcode_MAX_AGE);
		kforth_ops_add(kfops,	"MIN-AGE",		FindOpcode_MIN_AGE);
		kforth_ops_add(kfops,	"AVG-AGE",		FindOpcode_AVG_AGE);
		kforth_ops_add(kfops,	"MAX-NUM-CELLS",	FindOpcode_MAX_NUM_CELLS);



		/*
		 * copy to static table. (so clients of this data structure
		 * won't be required to free it)
		 */
		for(i=0; i < KFORTH_OPS_LEN; i++) {
			kfops_table[i] = kfops[i];
		}

		kforth_ops_delete(kfops);
	}

	return kfops_table;
}

