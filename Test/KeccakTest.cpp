#include "KeccakTest.h"
#include "../CEX/HMAC.h"
#include "../CEX/Keccak256.h"
#include "../CEX/Keccak512.h"
#include "../CEX/Keccak1024.h"
#include "../CEX/SymmetricKey.h"

//#define ENABLE_LONGKAT_TEST

namespace Test
{
	using namespace Digest;
	using CEX::Key::Symmetric::SymmetricKey;
	using CEX::Mac::HMAC;

	const std::string KeccakTest::DESCRIPTION = "Keccak Vector KATs; tests SHA-3 224/256/384/512 and HMACs.";
	const std::string KeccakTest::FAILURE = "FAILURE! ";
	const std::string KeccakTest::SUCCESS = "SUCCESS! All Keccak tests have executed succesfully.";

	KeccakTest::KeccakTest()
		:
		m_messages(0),
		m_expected256(0),
		m_expected512(0),
		m_macKeys(0),
		m_macData(0),
		m_mac256(0),
		m_mac512(0),
		m_progressEvent(),
		m_truncKey(0),
		m_truncData(0),
		m_trunc256(0),
		m_trunc512(0),
		m_xtremeData(0)
	{
	}

	KeccakTest::~KeccakTest()
	{
	}

	const std::string KeccakTest::Description()
	{
		return DESCRIPTION;
	}

	TestEventHandler &KeccakTest::Progress()
	{
		return m_progressEvent;
	}

	std::string KeccakTest::Run()
	{
		try
		{
			SHA3256KatTest();
			OnProgress(std::string("KeccakTest: Passed SHA3 256 bit digest vector tests.."));

			SHA3512KatTest();
			OnProgress(std::string("KeccakTest: Passed SHA3 512 bit digest vector tests.."));

			Keccak1024KatTest();
			OnProgress(std::string("KeccakTest: Passed Keccak 1024 bit digest vector tests.."));

			TreeParamsTest();
			OnProgress(std::string("KeccakTest: Passed KeccakParams parameter serialization test.."));

			return SUCCESS;
		}
		catch (TestException const &ex)
		{
			throw TestException(FAILURE + std::string(" : ") + ex.Message());
		}
		catch (...)
		{
			throw TestException(std::string(FAILURE + std::string(" : Unknown Error")));
		}
	}

	void KeccakTest::SHA3256KatTest()
	{
		std::vector<byte> output(32);

		std::vector<byte> msg0(0);
		std::vector<byte> msg24 = { 0x61, 0x62, 0x63 };
		std::vector<byte> msg448 = {
			0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67,
			0x65, 0x66, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6A, 0x68, 0x69, 0x6A, 0x6B,
			0x69, 0x6A, 0x6B, 0x6C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6B, 0x6C, 0x6D, 0x6E, 0x6C, 0x6D, 0x6E, 0x6F,
			0x6D, 0x6E, 0x6F, 0x70, 0x6E, 0x6F, 0x70, 0x71 };
		std::vector<byte> msg1600 = {
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3 };

		std::vector<byte> exp0 = {
			0xA7, 0xFF, 0xC6, 0xF8, 0xBF, 0x1E, 0xD7, 0x66, 0x51, 0xC1, 0x47, 0x56, 0xA0, 0x61, 0xD6, 0x62,
			0xF5, 0x80, 0xFF, 0x4D, 0xE4, 0x3B, 0x49, 0xFA, 0x82, 0xD8, 0x0A, 0x4B, 0x80, 0xF8, 0x43, 0x4A };
		std::vector<byte> exp24 = {
			0x3A, 0x98, 0x5D, 0xA7, 0x4F, 0xE2, 0x25, 0xB2, 0x04, 0x5C, 0x17, 0x2D, 0x6B, 0xD3, 0x90, 0xBD,
			0x85, 0x5F, 0x08, 0x6E, 0x3E, 0x9D, 0x52, 0x5B, 0x46, 0xBF, 0xE2, 0x45, 0x11, 0x43, 0x15, 0x32 };
		std::vector<byte> exp448 = {
			0x41, 0xC0, 0xDB, 0xA2, 0xA9, 0xD6, 0x24, 0x08, 0x49, 0x10, 0x03, 0x76, 0xA8, 0x23, 0x5E, 0x2C,
			0x82, 0xE1, 0xB9, 0x99, 0x8A, 0x99, 0x9E, 0x21, 0xDB, 0x32, 0xDD, 0x97, 0x49, 0x6D, 0x33, 0x76 };
		std::vector<byte> exp1600 = {
			0x79, 0xF3, 0x8A, 0xDE, 0xC5, 0xC2, 0x03, 0x07, 0xA9, 0x8E, 0xF7, 0x6E, 0x83, 0x24, 0xAF, 0xBF,
			0xD4, 0x6C, 0xFD, 0x81, 0xB2, 0x2E, 0x39, 0x73, 0xC6, 0x5F, 0xA1, 0xBD, 0x9D, 0xE3, 0x17, 0x87 };

		Keccak256 dgt(false);

		dgt.Update(msg0, 0, msg0.size());
		dgt.Finalize(output, 0);

		if (output != exp0)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Update(msg24, 0, msg24.size());
		dgt.Finalize(output, 0);

		if (output != exp24)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Compute(msg448, output);

		if (output != exp448)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Compute(msg1600, output);

		if (output != exp1600)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}
	}

	void KeccakTest::SHA3512KatTest()
	{
		std::vector<byte> output(64);

		std::vector<byte> msg0(0);
		std::vector<byte> msg24 = { 0x61, 0x62, 0x63 };
		std::vector<byte> msg448 = {
			0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67,
			0x65, 0x66, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6A, 0x68, 0x69, 0x6A, 0x6B,
			0x69, 0x6A, 0x6B, 0x6C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6B, 0x6C, 0x6D, 0x6E, 0x6C, 0x6D, 0x6E, 0x6F,
			0x6D, 0x6E, 0x6F, 0x70, 0x6E, 0x6F, 0x70, 0x71 };
		std::vector<byte> msg1600 = {
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3 };

		std::vector<byte> exp0 = {
			0xA6, 0x9F, 0x73, 0xCC, 0xA2, 0x3A, 0x9A, 0xC5, 0xC8, 0xB5, 0x67, 0xDC, 0x18, 0x5A, 0x75, 0x6E,
			0x97, 0xC9, 0x82, 0x16, 0x4F, 0xE2, 0x58, 0x59, 0xE0, 0xD1, 0xDC, 0xC1, 0x47, 0x5C, 0x80, 0xA6,
			0x15, 0xB2, 0x12, 0x3A, 0xF1, 0xF5, 0xF9, 0x4C, 0x11, 0xE3, 0xE9, 0x40, 0x2C, 0x3A, 0xC5, 0x58,
			0xF5, 0x00, 0x19, 0x9D, 0x95, 0xB6, 0xD3, 0xE3, 0x01, 0x75, 0x85, 0x86, 0x28, 0x1D, 0xCD, 0x26 };
		std::vector<byte> exp24 = {
			0xB7, 0x51, 0x85, 0x0B, 0x1A, 0x57, 0x16, 0x8A, 0x56, 0x93, 0xCD, 0x92, 0x4B, 0x6B, 0x09, 0x6E,
			0x08, 0xF6, 0x21, 0x82, 0x74, 0x44, 0xF7, 0x0D, 0x88, 0x4F, 0x5D, 0x02, 0x40, 0xD2, 0x71, 0x2E,
			0x10, 0xE1, 0x16, 0xE9, 0x19, 0x2A, 0xF3, 0xC9, 0x1A, 0x7E, 0xC5, 0x76, 0x47, 0xE3, 0x93, 0x40,
			0x57, 0x34, 0x0B, 0x4C, 0xF4, 0x08, 0xD5, 0xA5, 0x65, 0x92, 0xF8, 0x27, 0x4E, 0xEC, 0x53, 0xF0 };
		std::vector<byte> exp448 = {
			0x04, 0xA3, 0x71, 0xE8, 0x4E, 0xCF, 0xB5, 0xB8, 0xB7, 0x7C, 0xB4, 0x86, 0x10, 0xFC, 0xA8, 0x18,
			0x2D, 0xD4, 0x57, 0xCE, 0x6F, 0x32, 0x6A, 0x0F, 0xD3, 0xD7, 0xEC, 0x2F, 0x1E, 0x91, 0x63, 0x6D,
			0xEE, 0x69, 0x1F, 0xBE, 0x0C, 0x98, 0x53, 0x02, 0xBA, 0x1B, 0x0D, 0x8D, 0xC7, 0x8C, 0x08, 0x63,
			0x46, 0xB5, 0x33, 0xB4, 0x9C, 0x03, 0x0D, 0x99, 0xA2, 0x7D, 0xAF, 0x11, 0x39, 0xD6, 0xE7, 0x5E };
		std::vector<byte> exp1600 = {
			0xE7, 0x6D, 0xFA, 0xD2, 0x20, 0x84, 0xA8, 0xB1, 0x46, 0x7F, 0xCF, 0x2F, 0xFA, 0x58, 0x36, 0x1B,
			0xEC, 0x76, 0x28, 0xED, 0xF5, 0xF3, 0xFD, 0xC0, 0xE4, 0x80, 0x5D, 0xC4, 0x8C, 0xAE, 0xEC, 0xA8,
			0x1B, 0x7C, 0x13, 0xC3, 0x0A, 0xDF, 0x52, 0xA3, 0x65, 0x95, 0x84, 0x73, 0x9A, 0x2D, 0xF4, 0x6B,
			0xE5, 0x89, 0xC5, 0x1C, 0xA1, 0xA4, 0xA8, 0x41, 0x6D, 0xF6, 0x54, 0x5A, 0x1C, 0xE8, 0xBA, 0x00 };

		Keccak512 dgt(false);

		dgt.Update(msg0, 0, msg0.size());
		dgt.Finalize(output, 0);

		if (output != exp0)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Update(msg24, 0, msg24.size());
		dgt.Finalize(output, 0);

		if (output != exp24)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Compute(msg448, output);

		if (output != exp448)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Compute(msg1600, output);

		if (output != exp1600)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}
	}

	void KeccakTest::Keccak1024KatTest()
	{
		std::vector<byte> output(128);

		std::vector<byte> msg0(0);
		std::vector<byte> msg24 = { 0x61, 0x62, 0x63 };
		std::vector<byte> msg448 = {
			0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67,
			0x65, 0x66, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6A, 0x68, 0x69, 0x6A, 0x6B,
			0x69, 0x6A, 0x6B, 0x6C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6B, 0x6C, 0x6D, 0x6E, 0x6C, 0x6D, 0x6E, 0x6F,
			0x6D, 0x6E, 0x6F, 0x70, 0x6E, 0x6F, 0x70, 0x71 };
		std::vector<byte> msg1600 = {
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
			0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3 };

		std::vector<byte> exp0 = {
			0x88, 0x65, 0xE4, 0x19, 0x50, 0x9F, 0x0C, 0xFB, 0xB8, 0x36, 0x6F, 0x0A, 0xE6, 0x74, 0x2B, 0xCB,
			0x2B, 0x51, 0x9F, 0xB4, 0x90, 0xE2, 0xD0, 0xB6, 0x5E, 0x55, 0x3B, 0xBF, 0xAF, 0x10, 0x96, 0x31,
			0x4F, 0x85, 0xEA, 0x9D, 0x57, 0x19, 0x63, 0xAD, 0xF4, 0xFE, 0x17, 0x8C, 0x62, 0x40, 0x2A, 0xE4,
			0xC1, 0x9D, 0x89, 0x0C, 0x58, 0x54, 0x7A, 0x12, 0xA5, 0xEA, 0x54, 0xEE, 0x25, 0x6B, 0x92, 0x95,
			0x4F, 0x20, 0x25, 0x78, 0x29, 0xA5, 0x1A, 0x3F, 0x4A, 0xE0, 0x39, 0xD6, 0x99, 0xCA, 0x7D, 0xD2,
			0x80, 0x84, 0x9D, 0xE3, 0xCD, 0x0E, 0xFD, 0xF5, 0x3C, 0xC4, 0x30, 0x6D, 0x22, 0xD9, 0x81, 0x72,
			0xBE, 0x81, 0xD5, 0xA2, 0xED, 0x86, 0x4A, 0xF9, 0xFE, 0x66, 0x96, 0x2D, 0x25, 0xA9, 0x92, 0x21,
			0x2D, 0x18, 0x41, 0x49, 0x3D, 0x5B, 0x70, 0x5D, 0xDD, 0x9A, 0x70, 0x15, 0xB1, 0xD7, 0xF7, 0x7F };
		std::vector<byte> exp24 = {
			0xFD, 0xF6, 0xE6, 0x04, 0x57, 0x6A, 0xC8, 0x11, 0xED, 0x4C, 0x56, 0xB6, 0x22, 0xED, 0x96, 0xDB,
			0x05, 0xDB, 0x69, 0x00, 0x9C, 0xBE, 0x6B, 0xC1, 0xF3, 0xFD, 0x62, 0x90, 0xE2, 0x8D, 0xC4, 0x5E,
			0x61, 0x8C, 0x5B, 0x12, 0x1F, 0x21, 0xA1, 0x04, 0x00, 0x77, 0x63, 0xF4, 0x2A, 0x84, 0x5F, 0xE0,
			0x77, 0x17, 0xD5, 0x39, 0x7C, 0x92, 0x6E, 0x15, 0xC1, 0x35, 0x8A, 0x01, 0x45, 0xBA, 0xE1, 0x9A,
			0x6A, 0x9C, 0x68, 0x60, 0x95, 0xC6, 0xAE, 0xDD, 0xC8, 0x2A, 0x69, 0x4B, 0x82, 0x2B, 0xD7, 0x19,
			0x66, 0x11, 0xF6, 0xFF, 0x47, 0x09, 0x7D, 0x26, 0xFC, 0xCF, 0x6E, 0x6F, 0xC0, 0xA6, 0x2F, 0x43,
			0x3B, 0x61, 0xB8, 0x79, 0xB1, 0xE4, 0x55, 0xAE, 0xCE, 0xF8, 0xCB, 0xF0, 0x58, 0x77, 0xB0, 0x64,
			0x95, 0x1D, 0xF1, 0x91, 0xDF, 0x7C, 0x7F, 0x03, 0xB6, 0x50, 0xA7, 0xBA, 0x97, 0xDC, 0x36, 0x4C };
		std::vector<byte> exp448 = {
			0x7A, 0x24, 0x9A, 0x8B, 0x16, 0xA4, 0x98, 0x97, 0x22, 0x51, 0xB3, 0xE1, 0x50, 0x5A, 0xE7, 0x64,
			0x3E, 0x11, 0x29, 0x8D, 0x39, 0x06, 0xA9, 0x10, 0x9D, 0x8B, 0x88, 0x79, 0xC9, 0xFB, 0x27, 0x80,
			0x0A, 0x99, 0xE7, 0xD1, 0xE3, 0x5D, 0xAE, 0xBA, 0x15, 0xEC, 0x8E, 0x5F, 0x19, 0x70, 0x50, 0xEE,
			0xE0, 0x6A, 0x75, 0x4D, 0xA9, 0x3C, 0xA7, 0x34, 0x75, 0x63, 0x63, 0xDC, 0x7C, 0x71, 0x58, 0x7C,
			0x25, 0x32, 0x47, 0x9B, 0x27, 0xAD, 0x5C, 0x98, 0xB9, 0x43, 0x29, 0x33, 0x97, 0xAB, 0x0D, 0x18,
			0xAE, 0x2C, 0xEA, 0x76, 0x50, 0xE6, 0xF3, 0xF5, 0x76, 0x8E, 0xAC, 0x67, 0x24, 0x94, 0x3B, 0xBD,
			0x11, 0x8F, 0xF0, 0xD9, 0x0F, 0x09, 0xC5, 0x63, 0x91, 0xDA, 0xE1, 0x5F, 0x3F, 0x09, 0xD0, 0xD4,
			0x24, 0x80, 0xEA, 0x9F, 0x55, 0xCD, 0x1E, 0x2A, 0x30, 0x8C, 0xA9, 0x15, 0xE9, 0xD1, 0xF7, 0xCA };
		std::vector<byte> exp1600 = {
			0x8A, 0xB1, 0x91, 0xD9, 0x15, 0x93, 0x74, 0x01, 0xAD, 0x20, 0xEA, 0x29, 0x3A, 0x5C, 0xC1, 0x33,
			0xD7, 0x01, 0xE3, 0xD6, 0x83, 0x95, 0x89, 0xBF, 0x81, 0x7D, 0xE4, 0x97, 0x4B, 0x26, 0x36, 0xAC,
			0x9D, 0x90, 0x28, 0xBB, 0xE2, 0x19, 0xB6, 0x2F, 0x02, 0xCD, 0xB2, 0x86, 0x21, 0x82, 0xCD, 0x25,
			0x27, 0x12, 0xC4, 0x88, 0x6D, 0x71, 0x65, 0xF6, 0x27, 0xE3, 0xD4, 0x34, 0x87, 0xED, 0xBB, 0xD5,
			0x7E, 0xCE, 0x1F, 0x52, 0x8B, 0x7B, 0xF2, 0x14, 0xF0, 0x16, 0x8B, 0xA8, 0x9D, 0xDD, 0x91, 0x88,
			0x0A, 0x1E, 0xFE, 0xFF, 0x29, 0xAF, 0xB7, 0xEA, 0xFF, 0x3E, 0x62, 0xD5, 0xBC, 0xE4, 0x3D, 0x24,
			0xBA, 0x3A, 0x26, 0x59, 0xC2, 0x84, 0x3D, 0x22, 0xD6, 0xA1, 0x83, 0xC6, 0x8E, 0x74, 0x32, 0xF2,
			0x8C, 0x34, 0xDC, 0x25, 0x97, 0xB9, 0x58, 0xD8, 0x04, 0x52, 0xB2, 0x2F, 0x21, 0xAA, 0x9D, 0x40 };

		Keccak1024 dgt(false);

		dgt.Update(msg0, 0, msg0.size());
		dgt.Finalize(output, 0);

		if (output != exp0)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Update(msg24, 0, msg24.size());
		dgt.Finalize(output, 0);

		if (output != exp24)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Compute(msg448, output);

		if (output != exp448)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}

		dgt.Compute(msg1600, output);

		if (output != exp1600)
		{
			throw TestException("Keccak: Expected hash is not equal!");
		}
	}

	void KeccakTest::OnProgress(std::string Data)
	{
		m_progressEvent(Data);
	}

	void KeccakTest::TreeParamsTest()
	{
		std::vector<byte> code1(8, 7);

		KeccakParams tree1(32, 32, 8);
		tree1.DistributionCode() = code1;
		std::vector<byte> tres = tree1.ToBytes();
		KeccakParams tree2(tres);

		if (!tree1.Equals(tree2))
		{
			throw std::string("KeccakTest: Tree parameters test failed!");
		}

		std::vector<byte> code2(20, 7);
		KeccakParams tree3(0, 64, 1, 128, 8, 1, code2);
		tres = tree3.ToBytes();
		KeccakParams tree4(tres);

		if (!tree3.Equals(tree4))
		{
			throw std::string("KeccakTest: Tree parameters test failed!");
		}
	}
}