#!/bin/bash

(head -n1 output.csv; tail -fn0 output.csv) | polt add-source -p csv -o only-columns=Matches  live &
(head -n1 output.csv; tail -fn0 output.csv) | polt add-source -p csv -o only-columns=RecursiveCalls  live &
(head -n1 output.csv; tail -fn0 output.csv) | polt add-source -p csv -o only-columns=TotalTime  live &
(head -n1 output.csv; tail -fn0 output.csv) | polt add-source -p csv -o only-columns=SearchTime  live


# (head -n1 output.csv; tail -fn0 output.csv) | polt add-source -p csv live 
