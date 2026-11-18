# Owned Document query results

Search and Markdown conversion analysis return immutable native result owners.
A result can be retained independently of its snapshot and document. Search
ranges keep their original document identity and revision.

The new search uses the existing literal or flagged search with one traversal.
The conversion report uses the existing analysis and any required round-trip
check. Both grow result storage as needed and transfer it to the result owner.
An allocation failure releases the partial results. The legacy functions
continue to write into caller buffers and report their total counts.

Windows and Linux each passed 96 SDK checks covering O0/O2 clients with
hosted and static linkage, execution and import verification. The query
fixture covers 20 results, an empty result, scoped regex and whole-word
matching, legacy prefix agreement, shared ownership, eight failed search
allocation budgets and four failed conversion analysis allocation budgets.
