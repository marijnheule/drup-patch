# drup-patch
A patch to allow DRUP proof logging in MiniSAT based solvers

We implemented an improved checker, called <A href="https://github.com/marijnheule/drat-trim">DRAT-trim</A>,
which is backwards compatible with DRUP-trim, faster than DRUP-trim, and contains several new features.
For example, DRAT-trim can validate proofs with extended resolution. We advice users of DRUP-trim to
upgrade to DRAT-trim.
</b>
</td>
</table>
<BR><BR>
	<h1><a id="main">DRUP checker</a></h1>


<table>
<td>

Reverse unit propation (in short RUP) is a popular method to verify 
refutations produced by satisfiability (SAT) solvers. The idea is originates from [1].
RUP proofs can easily be obtained from most conflict-driven clause learning SAT solvers.
Certifying UNSAT proof tracks have been organized for the SAT competitions in 
<A href="http://www.satcompetition.org/2007/">2007</A>, 
<A href="http://www.satcompetition.org/2009/">2009</A>, and
<A href="http://www.satcompetition.org/2011/">2011</A>.
<BR>
<BR>
Probably the most important disadvantage of the RUP approach is the computational
costs to validate a given proof. In order to reduce this disadvantage, we propose
to add clause deletion information to the proof. Clause deletion is one of the
features that allow SAT solvers to have strong performance. By communicating
this information to a RUP proof checker, one can significantly reduce the costs
to validate a proof. We refer to our proposed format as DRUP (delete RUP). 
Details about the format are described below.

<BR><BR>
[1]   E.Goldberg, Y.Novikov.<BR>
  Verification of proofs of unsatisfiability for CNF formulas.<BR>
  Design, Automation and Test in Europe. March 2003, pp. 886--891.
</td>
</table>

<br>
<h2>Citing this work</h2>

<table>
<td>
If you would like to reference DRUP or DRUP-trim in a publication, please cite the following <a href="http://www.cs.utexas.edu/~marijn/publications/druptrim.pdf">paper</a>:
<br><br>
Heule, M.J.H., Hunt, Jr., W.A., and Wetzler, N.<BR>
Trimming while checking clausal proofs.<BR>
Formal Methods in Computer-Aided Design (FMCAD).  IEEE (2013) 181-188<BR><BR>
</td>
</table>

<h2>Downloads</h2>

<table>
<td>
We implemented two checkers for (D)RUP proofs:
<ul>
<li> The lastest version of our fast DRUP checker (<A href="drup-trim.c">drup-trim.c</A>). 
It combines backward RUP checking with watch literals to realize efficient performance. 
Additionally, DRUP-check can exploit clause deletion information in the proof. 
The description below explains how to add this information to a RUP proof.
<li> A compact implementation of a RUP checker (<A href="rup-forward.c">rup-forward.c</A>).
This checker is slightly more than 100 lines of code. In contrast to drup-check, rup-forward
does not support clause deletion information. Also, it implements forward checking which
can be useful for debugging purposes.
</ul>

Additionally, we implemented a patch for the solvers
<A href="https://www.lri.fr/~simon/downloads/glucose2.2.tgz">Glucose version 2.2</A> and 
<A href="http://minisat.se/downloads/minisat-2.2.0.tar.gz">Minisat version 2.2</A>.
These patches enable the solvers to emit a DRUP proof (for both the core and simp solvers).
To apply the <A href="DRUPglucose.patch">Glucose patch</A> (or <A href="DRUPminisat.patch">Minisat patch</A>), 
place it in the root directory of Glucose 2.2 (or Minisat 2.2) and run:<BR><BR>

patch -p1 < DRUPglucose.patch<BR><BR>

or for Minisat<BR><BR>

patch -p1 < DRUPminisat.patch<BR><BR>

After applying the patch, one can output a DRUP proof as follows:<BR><BR>

./glucose FORMULA PROOF<BR><BR>

or for Minisat<BR><BR>

./minisat FORMULA PROOF<BR><BR>

</td>
</table>

<h2>Usage</h2>

<table>
<td>
To make the executable, download the file above and compile it:
<BR><BR>
gcc drup-trim.c -O2 -o drup-trim
<BR><BR>
To run the checker:
<BR><BR>
./drup-trim FORMULA PROOF [CORE]
<BR><BR>
with FORMULA being a CNF formula in DIMACS format and PROOF a proof for FORMULA in the DRUP format (see details below). 
Additionally one can specify a file CORE containing the unsatisfiable core in DIMACS format. 
</td>
</table>

<h2>Example</h2>

<table>
<td>
Below, a brief description of the DRUP format based on an example formula. The spacing in the examples is to improve readability. Consider the following formula in the DIMACS format. 
</td>
</table>

<h2>
<pre>
p cnf 4 8
 1  2 -3 0
-1 -2  3 0
 2  3 -4 0
-2 -3  4 0
 1  3  4 0
-1 -3 -4 0
-1  2  4 0
 1 -2 -4 0
</pre>
</h2>

<table>
<td>
A compact RUP proof for the above formula is:
</td>
</table>

<h2>
<pre>
1 2 0
1 0
2 0
0
</pre>
</h2>

<table>
<td>
The method of deriving the redundant clauses in the proof is not important.
It might be by resolution or some other means.
It is only necessary that the soundness of the redundant clauses can be
verified by a procedure called REVERSE UNIT-PROPAGATION (RUP for
short).<BR>
In the discussion, clauses are delimited by square brackets.

Suppose that F is the input formula and R_1, ..., R_r are the
redundant clauses that have been produced by the solver, with R_r = [] (the empty clause).
The sequence R_1, ..., R_r is an RUP refutation of F if and only if:
<BR><BR>
For each i from 1 through r, steps 1--3 below derive []:
<ol>
<li>Negate R_i = [L_{ij}] producing one or more unit clauses, [-L_{ij}].<BR>
      For example, the negation of [x, y, z] is [-x], [-y], [-z].<BR>
      (When i = r, there are no unit clauses produced.)
<li>Add these unit clauses [-L_{ij}] and R_1 through R_{i-1} to F.<BR>
      (When i = r, there are no unit clauses to add.)
<li>Perform unit-clause propagation.
</ol>
The conflict clauses clauses produced by conflict-driven clause learning (CDCL) solvers have the RUP property. 
Therefore, one can construct a RUP proof for unsatisfiable formulas by listing all conflict clauses 
(in the order of learning). 
<BR><BR>
One important disadvantage of checking RUP proofs is the cost to verify a proof. To counter this disadvantage, we propose the DRUP format (delete reverse unit propagation). The DRUP format extends the RUP format by allowing it to express clause elimination information within the proof format.
</td>
</table>

<h2>
<pre>
   1  2  0
d  1  2 -3 0
   1  0
d  1  2  0
d  1  3  4 0
d  1 -2 -4 0
   2  0
   0
</pre>
</h2>

<table>
<td>
Apart from the redundant RUP clauses, a DRUP proof may contain lines with a 'd' prefix. These lines refer to clauses (original or learned) that can be deleted without influencing the proof checking. In the above example, all the deleted clauses are subsumed by added RUP clauses. In practice, however, the clauses that one wants to include in a DRUP proof are the clauses that are removed while reducing the (learned) clause database.
