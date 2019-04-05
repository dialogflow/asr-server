#!/bin/sh

# This is the special lang/graph which only has the words in the stories:
special_lang=/home/ubuntu/kaldi1/egs/amira/s5/data/lang_phone_nslibri
text=/home/ubuntu/kaldi1/egs/amira/s5/new-alts/text
graphs=/home/ubuntu/kaldi1/egs/amira/s5/new-alts/graphs.scp
#text=/home/ubuntu/kaldi1/egs/amira/s5/text
#graphs=/home/ubuntu/kaldi1/egs/amira/s5/graphs.scp

model=/home/ubuntu/kaldi1/egs/amira/s5/exp/chain/libri_noiv
# This is a generic graph
lang=/home/ubuntu/kaldi1/egs/amira/s5/exp/chain/libri_noiv/graph_phone_bg

# NOTE:
# The phones.txt and phones/word_boundary_info.int in the two langs should
# be the same. Otherwise, the outcome will be undefined.

if ! diff $lang/phones/word_boundary.int $special_lang/phones/word_boundary.int; then
  echo "Langs not compatible.";
  exit 0;
fi

word_boundary=$lang/phones/word_boundary.int
./fcgi-nnet3-decoder --default-graph=$lang/HCLG.fst --default-word-symbol-table=$lang/words.txt \
                     --word-symbol-table=$special_lang/words.txt \
                     --lm-scale=1.0 --word-boundary-info=$word_boundary \
                     --nnet-in=$model/final.mdl \
                     --fcgi-socket=:5009 --fcgi-threads-number=8
