#include "HKDFTest.h"
#include "../CEX/HKDF.h"
#include "../CEX/HMAC.h"
#include "../CEX/IntegerTools.h"
#include "../CEX/SecureRandom.h"
#include "../CEX/SHA256.h"
#include "../CEX/SymmetricKeySize.h"

namespace Test
{
	using Exception::CryptoKdfException;
	using Kdf::HKDF;
	using Mac::HMAC;
	using Utility::IntegerTools;
	using Prng::SecureRandom;
	using Digest::SHA256;
	using Enumeration::SHA2Digests;
	using Cipher::SymmetricKeySize;

	const std::string HKDFTest::CLASSNAME = "HKDFTest";
	const std::string HKDFTest::DESCRIPTION = "HKDF RFC 5869 SHA-2 test vectors.";
	const std::string HKDFTest::SUCCESS = "SUCCESS! All HKDF tests have executed succesfully.";

	HKDFTest::HKDFTest()
		: 
		m_expected(0),
		m_info(0),
		m_key(0),
		m_progressEvent(),
		m_salt(0)
	{
		Initialize();
	}

	HKDFTest::~HKDFTest()
	{
		IntegerTools::Clear(m_expected);
		IntegerTools::Clear(m_info);
		IntegerTools::Clear(m_key);
		IntegerTools::Clear(m_salt);
	}

	const std::string HKDFTest::Description()
	{
		return DESCRIPTION;
	}

	TestEventHandler &HKDFTest::Progress()
	{
		return m_progressEvent;
	}

	std::string HKDFTest::Run()
	{
		try
		{
			Exception();
			OnProgress(std::string("HKDFTest: Passed HKDF exception handling tests.."));

			HKDF* gen1 = new HKDF(SHA2Digests::SHA256);
			Kat(gen1, m_key[0], m_salt[0], m_info[0], m_expected[0]);
			Kat(gen1, m_key[1], m_salt[1], m_info[1], m_expected[1]);
			OnProgress(std::string("HKDFTest: Passed HKDF SHA2-256 known answer tests.."));

			HKDF* gen2 = new HKDF(SHA2Digests::SHA256);
			Kat(gen2, m_key[0], m_salt[0], m_info[0], m_expected[2]);
			Kat(gen2, m_key[1], m_salt[1], m_info[1], m_expected[3]);
			OnProgress(std::string("HKDFTest: Passed HKDF SHA2-512 known answer tests.."));

			Params(gen1);
			Params(gen2);
			OnProgress(std::string("HKDFTest: Passed initialization parameters tests.."));

			Stress(gen1);
			Stress(gen2);
			OnProgress(std::string("HKDFTest: Passed stress tests.."));

			delete gen1;
			delete gen2;

			return SUCCESS; 
		}
		catch (TestException const &ex)
		{
			throw TestException(CLASSNAME, ex.Function(), ex.Origin(), ex.Message());
		}
		catch (CryptoKdfException &ex)
		{
			throw TestException(CLASSNAME, ex.Location() + std::string("::") + ex.Origin(), ex.Name(), ex.Message());
		}
		catch (CryptoException &ex)
		{
			throw TestException(CLASSNAME, ex.Location() + std::string("::") + ex.Origin(), ex.Name(), ex.Message());
		}
		catch (std::exception const &ex)
		{
			throw TestException(CLASSNAME, std::string("Unknown Origin"), std::string(ex.what()));
		}
	}

	void HKDFTest::Exception()
	{
		// test constructor
		try
		{
			// invalid digest choice
			HKDF gen(SHA2Digests::None);

			throw TestException(std::string("Exception"), gen.Name(), std::string("Exception handling failure! -HE1"));
		}
		catch (CryptoKdfException const &)
		{
		}
		catch (TestException const &)
		{
			throw;
		}

		// test initialization
		try
		{
			HKDF gen(SHA2Digests::SHA256);
			// invalid key size
			std::vector<byte> key(1);
			SymmetricKey kp(key);
			gen.Initialize(kp);

			throw TestException(std::string("Exception"), gen.Name(), std::string("Exception handling failure! -HE2"));
		}
		catch (CryptoKdfException const &)
		{
		}
		catch (TestException const &)
		{
			throw;
		}

		// test generator state -1
		try
		{
			HKDF gen(SHA2Digests::SHA256);
			std::vector<byte> otp(32);
			// generator was not initialized
			gen.Generate(otp);

			throw TestException(std::string("Exception"), gen.Name(), std::string("Exception handling failure! -HE3"));
		}
		catch (CryptoKdfException const &)
		{
		}
		catch (TestException const &)
		{
			throw;
		}

		// test generator state -2
		try
		{
			HKDF gen(SHA2Digests::SHA256);
			Cipher::SymmetricKeySize ks = gen.LegalKeySizes()[1];
			std::vector<byte> key(ks.KeySize());
			std::vector<byte> otp(32);
			SymmetricKey kp(key);
			gen.Initialize(kp);
			// array too small
			gen.Generate(otp, 0, otp.size() + 1);

			throw TestException(std::string("Exception"), gen.Name(), std::string("Exception handling failure! -HE4"));
		}
		catch (CryptoKdfException const &)
		{
		}
		catch (TestException const &)
		{
			throw;
		}

		// test generator state -3
		try
		{
			HKDF gen(SHA2Digests::SHA256);
			Cipher::SymmetricKeySize ks = gen.LegalKeySizes()[1];
			std::vector<byte> key(ks.KeySize());
			// output exceeds maximum
			std::vector<byte> otp(256 * 32);
			SymmetricKey kp(key);
			gen.Initialize(kp);
			gen.Generate(otp, 0, otp.size());

			throw TestException(std::string("Exception"), gen.Name(), std::string("Exception handling failure! -HE5"));
		}
		catch (CryptoKdfException const &)
		{
		}
		catch (TestException const &)
		{
			throw;
		}
	}

	void HKDFTest::Kat(IKdf* Generator, std::vector<byte> &Key, std::vector<byte> &Salt, std::vector<byte> &Info, std::vector<byte> &Expected)
	{
		std::vector<byte> otp(Expected.size());
		SymmetricKey kp(Key, Salt, Info);
		Generator->Initialize(kp);
		Generator->Generate(otp);

		if (otp != Expected)
		{
			throw TestException(std::string("Kat"), Generator->Name(), std::string("Output does not match the known answer! -HK1"));
		}
	}

	void HKDFTest::Initialize()
	{
		/*lint -save -e122 */
		/*lint -save -e146 */
		/*lint -save -e417 */
		const std::vector<std::string> keys =
		{
			std::string("0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B"),
			std::string("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F404142434445464748494A4B4C4D4E4F")
		};
		HexConverter::Decode(keys, 2, m_key);

		const std::vector<std::string> salt =
		{
			std::string("000102030405060708090A0B0C"),
			std::string("606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F808182838485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9FA0A1A2A3A4A5A6A7A8A9AAABACADAEAF")
		};
		HexConverter::Decode(salt, 2, m_salt);

		const std::vector<std::string> info =
		{
			std::string("F0F1F2F3F4F5F6F7F8F9"),
			std::string("B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBFC0C1C2C3C4C5C6C7C8C9CACBCCCDCECFD0D1D2D3D4D5D6D7D8D9DADBDCDDDEDFE0E1E2E3E4E5E6E7E8E9EAEBECEDEEEFF0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF")
		};
		HexConverter::Decode(info, 2, m_info);

		const std::vector<std::string> expected =
		{
			// official vectors
			std::string("3CB25F25FAACD57A90434F64D0362F2A2D2D0A90CF1A5A4C5DB02D56ECC4C5BF34007208D5B887185865"),
			std::string("B11E398DC80327A1C8E7F78C596A49344F012EDA2D4EFAD8A050CC4C19AFA97C59045A99CAC7827271CB41C65E590E09DA3275600C2F09B8367793A9ACA3DB71CC30C58179EC3E87C14C01D5C1F3434F1D87"),
			// sha512 original vectors
			std::string("3CB25F25FAACD57A90434F64D0362F2A2D2D0A90CF1A5A4C5DB02D56ECC4C5BF34007208D5B887185865B4B0A85A993B89B9B65683D60F0106D28FFF039D0B6F"),
			std::string("B11E398DC80327A1C8E7F78C596A49344F012EDA2D4EFAD8A050CC4C19AFA97C59045A99CAC7827271CB41C65E590E09DA3275600C2F09B8367793A9ACA3DB71CC30C58179EC3E87C14C01D5C1F3434F1D8783C7310A8770344B9D6050FE8772CE16AFE7DBB994F8FB72F5DF08C703099E1E3D594A326836564CF64914338F0D")
		};
		HexConverter::Decode(expected, 4, m_expected);
		/*lint -restore */
	}

	void HKDFTest::OnProgress(const std::string &Data)
	{
		m_progressEvent(Data);
	}

	void HKDFTest::Params(IKdf* Generator)
	{
		SymmetricKeySize ks = Generator->LegalKeySizes()[1];
		std::vector<byte> otp1;
		std::vector<byte> otp2;
		std::vector<byte> key(ks.KeySize());
		SecureRandom rnd;
		size_t i;

		otp1.reserve(MAXM_ALLOC);
		otp2.reserve(MAXM_ALLOC);

		for (i = 0; i < TEST_CYCLES; ++i)
		{
			const size_t OTPLEN = static_cast<size_t>(rnd.NextUInt32(MAXM_ALLOC, MINM_ALLOC));
			otp1.resize(OTPLEN);
			otp2.resize(OTPLEN);
			IntegerTools::Fill(key, 0, key.size(), rnd);

			// generate with the gen
			SymmetricKey kp(key);
			Generator->Initialize(kp);
			Generator->Generate(otp1, 0, OTPLEN);
			Generator->Reset();
			Generator->Initialize(kp);
			Generator->Generate(otp2, 0, OTPLEN);

			if (otp1 != otp2)
			{
				throw TestException(std::string("Params"), Generator->Name(), std::string("Returns a different array after reset! -HR1"));
			}
		}
	}

	void HKDFTest::Stress(IKdf* Generator)
	{
		SymmetricKeySize ks = Generator->LegalKeySizes()[1];
		std::vector<byte> otp;
		std::vector<byte> key(ks.KeySize());
		SecureRandom rnd;
		size_t i;

		otp.reserve(MAXM_ALLOC);

		for (i = 0; i < TEST_CYCLES; ++i)
		{
			try
			{
				const size_t OTPLEN = static_cast<size_t>(rnd.NextUInt32(MAXM_ALLOC, MINM_ALLOC));
				otp.resize(OTPLEN);
				IntegerTools::Fill(key, 0, key.size(), rnd);

				// generate with the kdf
				SymmetricKey kp(key);
				Generator->Initialize(kp);
				Generator->Generate(otp, 0, OTPLEN);
				Generator->Reset();
			}
			catch (CryptoException&)
			{
				throw;
			}
			catch (std::exception const&)
			{
				throw TestException(std::string("Stress"), Generator->Name(), std::string("The generator has thrown an exception! -HS1"));
			}
		}
	}
}
