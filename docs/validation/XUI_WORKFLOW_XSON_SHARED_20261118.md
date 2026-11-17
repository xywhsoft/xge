# Workflow XSON with shared values

`xuiWorkflowToXValueShared` creates the ordinary XSON envelope and takes
independent references to node configurations and variable defaults. The
envelope stays valid after the model closes. Configuration and default
values keep their identity so a caller can snapshot the complete result
once and preserve repeated references within that snapshot.

`xuiWorkflowLoadXValueShared` validates and prepares the new graph state
and variable table before updating the existing model. Configuration and
default values are retained from the input envelope. Existing graph
references and node type registrations keep their identity. Both methods
share the checked Workflow operation scope and remain creator-affine.
The original export and load methods continue to use native deep copies.

The CPU-only source fixture checks shared identity, copy isolation, model
survival and every allocation failure up to the first success. GCC C11
O0 and O2 pass on Windows and Linux with matching xlang XRT. The measured
failure prefixes for this fixture are 72 export and 12 load allocations.
