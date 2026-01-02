import m5, m5.objects as m5o
print([n for n in dir(m5o) if n.endswith('RP') or 'ReplacementPolicy' in n])

# ['BIPRP', 'BRRIPRP', 'BaseReplacementPolicy', 'DRRIPRP', 'DuelingRP', 'FIFORP', 'LFURP', 'LIPRP', 'LRURP', 'MRURP', 'NRURP', 'RRIPRP', 'RandomRP', 'SHiPMemRP', 'SHiPPCRP', 'SHiPRP', 'SecondChanceRP', 'TreePLRURP', 'WeightedLRURP']