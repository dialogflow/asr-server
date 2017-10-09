
all:

include ../apiai.mk
include $(KALDI_PATH)/kaldi.mk

LDFLAGS += $(CUDA_LDFLAGS)
LDLIBS += -lfcgi -lfcgi++ $(CUDA_LDLIBS)
EXTRA_CXXFLAGS += -I$(KALDI_PATH) -L$(KALDI_PATH) $(APIAI_CXX_FLAGS)

OBJFILES = Timing.o Response.o RequestRawReader.o ResponseJsonWriter.o ResponseMultipartJsonWriter.o OnlineDecoder.o Nnet3LatgenFasterDecoder.o QueryStringParser.o FcgiDecodingApp.o 

LIBNAME = libstidecoder

BINFILES = fcgi-nnet3-decoder

TESTFILES = QueryStringParserTests

ADDLIBS = $(KALDI_PATH)/online2/kaldi-online2.a $(KALDI_PATH)/ivector/kaldi-ivector.a \
          $(KALDI_PATH)/nnet2/kaldi-nnet2.a $(KALDI_PATH)/nnet3/kaldi-nnet3.a $(KALDI_PATH)/lat/kaldi-lat.a \
          $(KALDI_PATH)/decoder/kaldi-decoder.a  $(KALDI_PATH)/cudamatrix/kaldi-cudamatrix.a \
          $(KALDI_PATH)/feat/kaldi-feat.a $(KALDI_PATH)/transform/kaldi-transform.a $(KALDI_PATH)/gmm/kaldi-gmm.a \
          $(KALDI_PATH)/hmm/kaldi-hmm.a $(KALDI_PATH)/tree/kaldi-tree.a \
          $(KALDI_PATH)/matrix/kaldi-matrix.a $(KALDI_PATH)/fstext/kaldi-fstext.a \
          $(KALDI_PATH)/util/kaldi-util.a $(KALDI_PATH)/base/kaldi-base.a 
          
include $(KALDI_PATH)/makefiles/default_rules.mk
