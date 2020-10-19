// rup-forward.c   last edit: March 17, 2013

#include <stdio.h>

#define END      -9
#define MEM_MAX   1000000000
#define UNSAT     0
#define SAT       1

struct solver { FILE *file; int *DB, nVars, nClauses, mem_used, *falseStack, *false, *first,
                *forced, *processed, *assigned; FILE *proofFile; };

#define ASSIGN(a)	{ S->false[-(a)] = 1; *(S->assigned++) = -(a); }
#define ADD_WATCH(l,m)  { S->DB[(m)] = S->first[(l)]; S->first[(l)] = (m); }

int* getMemory (struct solver *S, int mem_size) {
  if (S->mem_used + mem_size > MEM_MAX) printf("Out of Memory\n");
  int *store = (S->DB + S->mem_used);
  S->mem_used += mem_size;
  return store; }

int* addClause (struct solver *S, int* input, int size) {
  if (size > 1) { ADD_WATCH (input[0], S->mem_used); ADD_WATCH (input[1], S->mem_used + 1); }
  int i, *clause = getMemory (S, size + 3) + 2;
  for (i = 0; i < size; ++i) { clause[ i ] = input[ i ]; } clause[ i ] = 0;
  return clause; }

int propagate (struct solver* S) {                 // Performs unit propagation
  while (S->processed < S->assigned) {             // While unprocessed false literals
    int  lit   = *(S->processed++);                // Get first unprocessed literal
    int* watch = &S->first[ lit ];                 // Obtain the first watch pointer
    while (*watch != END) {                        // While there are watched clauses (watched by lit)
      int i, *clause = (S->DB + *watch + 1);	   // Get the clause from DB
      if (!clause[-2]) clause++;                   // Set the pointer to the first literal in the clause
      if (clause[0] == lit) clause[0] = clause[1]; // Ensure that the other watched literal is in front
      for (i = 2; clause[i]; ++i)                  // Scan the non-watched literals
        if (S->false[ clause[i] ] == 0) {          // When clause[j] is not false, it is either true or unset
          clause[1] = clause[i]; clause[i] = lit;  // Swap literals
          int store = *watch;                      // Store the old watch
          *watch =  S->DB[ *watch ];               // Remove the watch from the list of lit
          ADD_WATCH (clause[1], store);            // Add the watch to the list of clause[1]
          goto next_clause; }                      // Goto the next watched clause
      clause[1] = lit; watch = (S->DB + *watch);   // Set lit at clause[1] and set next watch
      if ( S->false[ -clause[0] ]) continue; 	   // If the other watched literal is satisfied continue
      if (!S->false[  clause[0] ]) {               // If the other watched literal is falsified,
        ASSIGN (clause[0]); }                      // A unit clause is found, and the reason is set
      else return UNSAT;                           // Found a root level conflict -> UNSAT
      next_clause: ; } }                           // Set position for next clause
  return SAT; }	                                   // Finally, no conflict was found

int verify (struct solver *S) {
  int buffer [S->nVars];

  if (propagate (S) == UNSAT) return UNSAT; S->forced = S->processed;

  for (;;) {
    start:;
    int flag = 0, size = 0, tmp = 0, lit;
    printf("checking: ");
    while (tmp >= 0) {
      tmp = fscanf (S->proofFile, " %i ", &lit);    // Read a literal.
      if (!lit) break;
      if ( S->false[ -lit ]) flag = 1;
      if (!S->false[  lit ]) {
      printf("%i ", lit);
      buffer[ size++ ] = lit; } }  // Assign literal and add literal to buffer

    if (flag) goto start;

    int i; for (i = 0; i < size; ++i) ASSIGN(-buffer[i]);
    if (propagate (S) == SAT) return SAT;
    printf("verified\n");

    while (S->assigned > S->forced) S->false[ (*(--S->assigned)) ] = 0;
    S->processed = S->forced;

    if (size == 1) { ASSIGN (buffer[0]); if (propagate (S) == UNSAT) return UNSAT; S->forced = S->processed; }
    else addClause (S, buffer, size);
  }
  return SAT; }

int parse (struct solver* S) {
  int tmp, lines;
  do { tmp = fscanf (S->file, " p cnf %i %i \n", &S->nVars, &S->nClauses); // Read the first line
    if (tmp > 0 && tmp != EOF) break; tmp = fscanf (S->file, "%*s\n"); }   // In case a commment line was found
  while (tmp != 2 && tmp != EOF);                                          // Skip it and read next line
  int nZeros = S->nClauses, buffer [S->nVars], size = 0, n = S->nVars;     // Make a local buffer

  S->mem_used   = 0;                  // The number of integers allocated in the DB
  S->falseStack = getMemory (S, n+1); // Stack of falsified literals -- this pointer is never changed
  S->forced     = S->falseStack;      // Points inside *falseStack at first decision (unforced literal)
  S->processed  = S->falseStack;      // Points inside *falseStack at first unprocessed literal
  S->assigned   = S->falseStack;      // Points inside *falseStack at last unprocessed literal
  S->false      = getMemory (S, 2*n+1); S->false += n; // Labels for variables, non-zero means false
  S->first      = getMemory (S, 2*n+1); S->first += n; // Offset of the first watched clause
  S->DB[ S->mem_used++ ] = 0;         // Make sure there is a 0 before the clauses are loaded.

  int i; for (i = 1; i <= n; ++i) { S->false[i] = S->false[-i] = 0; S->first[i] = S->first[-i] = END; }

  while (nZeros > 0) {
    int lit, *clause; tmp = fscanf (S->file, " %i ", &lit);  // Read a literal.
    if (!lit) { clause = addClause (S, buffer, size);        // Add clause to data_base
      if (!size || ((size == 1) && S->false[ clause[0] ]))   // Check for empty clause or conflicting unit
        return UNSAT;                                        // If either is found return UNSAT
      if ((size == 1) && !S->false[ -clause[0] ]) {          // Check for a new unit
        ASSIGN (clause[0]); }                                // Directly assign new units
      size = 0; --nZeros; }                                  // Reset buffer
    else buffer[ size++ ] = lit; }                           // Add literal to buffer

  return SAT; }

int memory[ MEM_MAX ];

int main (int argc, char** argv) {
  struct solver S; S.DB = memory;
  S.file = fopen (argv[1], "r");
  if (S.file == NULL) {
    printf("Error opening \"%s\".\n", argv[1]);
    return 0;
  }
  S.proofFile = fopen (argv[2], "r");
  if (S.proofFile == NULL) {
    printf("Error opening \"%s\".\n", argv[2]);
    fclose (S.file);
    return 0;
  }
  if       (parse  (&S) == UNSAT) printf("s TRIVIAL UNSAT\n");
  else if  (verify (&S) == UNSAT) printf("s VERIFIED\n");
  else                            printf("s NOT VERIFIED\n")  ;
  fclose (S.file);
  fclose (S.proofFile);
  return 0;
}
