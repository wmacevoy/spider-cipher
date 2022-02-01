# spider-cipher

A modern cipher based on cards and dice.

# Command line

bin/spider-cipher [--decimal|--base40|--cards] --key='key message' --encrypt='plain' --decrypt='cipher'

# BUILD

```sh
make all
```

# TEST

## All tests (week of runtime, 16 GB ram, 200 GB disk space)

```sh
make all && nice time bin/spider_cipher_facts --facts_include='*' 2>&1 | tee "run-$(date).log"
 ```

## Most tests (few minutes of runtime, 4 GB ram, no disk space):

```sh
make all && nice time bin/spider_cipher_facts 2>&1 | tee "run-$(date).log"
 ```



 