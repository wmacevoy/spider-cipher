# spider-solitaire-cipher

visible cipher

# Command line

bin/spider-solitaire [--decimal|--base40|--cards] --key='key message' --encrypt='plain' --decrypt='cipher'

# BUILD

```sh
make all
```

# TEST

## All tests (week of runtime, 16 GB ram, 200 GB disk space)

```sh
make all && nice time bin/spider_solitaire_facts --facts_include='*' 2>&1 | tee "run-$(date).log"
 ```

## Most tests (few minutes of runtime, 4 GB ram, no disk space):

```sh
make all && nice time bin/spider_solitaire_facts 2>&1 | tee "run-$(date).log"
 ```



 