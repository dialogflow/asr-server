/*
 * QueryStringParser.cpp
 *
 *  Created on: Apr 6, 2016
 *      Author: folomeev
 */

#include "QueryStringParser.h"
#include "base/kaldi-error.h"

namespace apiai {

	void TestEmptyString() {
		std::string name, value;
		QueryStringParser parser("");

		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestQuestionMarkString() {
		std::string name, value;
		QueryStringParser parser("?");

		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestSingleNameValue() {
		std::string name, value;
		QueryStringParser parser("?name=value");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "name");
		KALDI_ASSERT(value == "value");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestSingleNameNoValue() {
		std::string name, value;
		QueryStringParser parser("?name=");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "name");
		KALDI_ASSERT(value == "");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestSingleNoNameValue() {
		std::string name, value;
		QueryStringParser parser("?=value");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "");
		KALDI_ASSERT(value == "value");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestEndsWithAmpersand() {
		std::string name, value;
		QueryStringParser parser("?&");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "");
		KALDI_ASSERT(value == "");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestEquatationInValue() {
		std::string name, value;
		QueryStringParser parser("?name=v=u");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "name");
		KALDI_ASSERT(value == "v=u");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}


	void TestSingleNoNameNoValue() {
		std::string name, value;
		QueryStringParser parser("?=");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "");
		KALDI_ASSERT(value == "");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestTwoNameValuePairs() {
		std::string name, value;
		QueryStringParser parser("?name1=value1&name2=value2");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "name1");
		KALDI_ASSERT(value == "value1");
		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "name2");
		KALDI_ASSERT(value == "value2");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}

	void TestEmptyNameAfterEquality() {
		std::string name, value;
		QueryStringParser parser("?name1=value1&=value2");

		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "name1");
		KALDI_ASSERT(value == "value1");
		KALDI_ASSERT(parser.HasNext());
		KALDI_ASSERT(parser.Next(&name, &value));
		KALDI_ASSERT(name == "");
		KALDI_ASSERT(value == "value2");
		KALDI_ASSERT(!parser.HasNext());
		KALDI_ASSERT(!parser.Next(&name, &value));
	}


} /* namespace apiai */



int main(int argn, char *argv[]) {
	using namespace apiai;

	TestEmptyString();
	TestQuestionMarkString();
	TestEndsWithAmpersand();
	TestEquatationInValue();
	TestSingleNameValue();
	TestSingleNameNoValue();
	TestSingleNoNameValue();
	TestSingleNoNameNoValue();
	TestTwoNameValuePairs();
	TestEmptyNameAfterEquality();
	return 0;
}

