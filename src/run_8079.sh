#!/bin/sh

# Decoding Beam = 18    Port = 8079

# This is the special lang/graph which only has the words in the stories:
special_lang=/home/ubuntu/kaldi1/egs/amira/s5/data/lang_all_nslibri
text=/home/ubuntu/kaldi1/egs/amira/s5/new-alts/text
graphs=/home/ubuntu/kaldi1/egs/amira/s5/new-alts/graphs.scp
#text=/home/ubuntu/kaldi1/egs/amira/s5/text
#graphs=/home/ubuntu/kaldi1/egs/amira/s5/graphs.scp

model=/home/ubuntu/fcgi-server/models/tdnn_1d_noiv_sp_amira_v2/
# This is a generic graph
lang=$model/graph_tgsmall

# NOTE:
# The phones.txt and phones/word_boundary_info.int in the two langs should
# be the same. Otherwise, the outcome will be undefined.

if ! diff $lang/phones/word_boundary.int $special_lang/phones/word_boundary.int; then
  echo "Langs not compatible.";
  exit 0;
fi

word_boundary=$lang/phones/word_boundary.int
/home/ubuntu/fcgi-server/src/fcgi-nnet3-decoder --mfcc-config=/home/ubuntu/fcgi-server/src/mfcc.conf \
                                                --default-graph=$lang/HCLG.fst --default-word-symbol-table=$lang/words.txt \
                                                --word-symbol-table=$special_lang/words.txt --trans-list=ark:$text \
                                                --graph-list=scp:$graphs --lm-scale=1.0 --word-boundary-info=$word_boundary \
                                                --nnet-in=$model/final.mdl --beam=18.0 \
                                                --fcgi-socket=:8079 --fcgi-threads-number=8
