# spider-solitaire

# Run Test

## all tests (week of runtime, 16 GB ram, 200 GB disk space)

```bash
make all && nice time bin/spider_solitaire_facts.exe --facts_include='*' 2>&1 | tee "run-$(date).log"
 ```

## most tests (hour of runtime, 4 GB ram, no disk space):

```bash
make all && nice time bin/spider_solitaire_facts.exe 2>&1 | tee "run-$(date).log"
 ```



 