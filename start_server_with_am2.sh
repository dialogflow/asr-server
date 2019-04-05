#!/bin/sh
BASE="/home/kaldi/amira"
GRAPHS_DIR="$BASE/lm/asr-am2-85-amira-cmu/graphs.scp"
PORT=8000
NNET="$BASE/am/amira-acoustic-model-v2/final.mdl"
SPECIAL_LANG_DIR="$BASE/lang/amira_lang_04022019"
TEXT="$BASE/lm/asr-am2-85-amira-cmu/text"

cd /home/kaldi/fcgi-server/src

# This is the special lang/graph which only has the words in the stories:
special_lang=$SPECIAL_LANG_DIR
text=$TEXT
graphs=$GRAPHS_DIR

# This is a generic graph
lang=$BASE/lm/default/models/tdnn_1d_noiv_sp_amira_v2/graph_tgsmall

# NOTE:
# The phones.txt and phones/word_boundary_info.int in the two langs should
# be the same. Otherwise, the outcome will be undefined.

if ! diff $lang/phones/word_boundary.int $special_lang/phones/word_boundary.int; then
  echo "Langs not compatible.";
  exit 0;
fi

word_boundary=$lang/phones/word_boundary.int
./fcgi-nnet3-decoder --default-graph=$BASE/lm/default/models/tdnn_1d_noiv_sp_amira_v2/graph_tgsmall/HCLG.fst --default-word-symbol-table=$BASE/lm/default/models/tdnn_1d_noiv_sp_amira_v2/graph_tgsmall/words.txt \
		--nnet-in=$NNET --word-symbol-table=$special_lang/words.txt --trans-list=ark:$text \
                     --graph-list=scp:$graphs --lm-scale=1.0 --word-boundary-info=$word_boundary \
                     --fcgi-socket=:$PORT --fcgi-threads-number=8

