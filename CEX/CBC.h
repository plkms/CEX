﻿// The GPL version 3 License (GPLv3)
// 
// Copyright (c) 2017 vtdev.com
// This file is part of the CEX Cryptographic library.
// 
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.
// 
//
// Implementation Details:
// An implementation of a Cipher Block Chaining Mode (CBC).
// Written by John Underhill, September 24, 2014
// Updated September 1, 2016
// Contact: develop@vtdev.com

#ifndef _CEX_CBC_H
#define _CEX_CBC_H

#include "ICipherMode.h"

NAMESPACE_MODE

/// <summary>
/// An implementation of a Cipher Block Chaining Mode
/// </summary> 
/// 
/// <example>
/// <description>Encrypting a single block of bytes:</description>
/// <code>
/// CBC cipher(BlockCiphers::AHX);
/// // initialize for encryption
/// cipher.Initialize(true, SymmetricKey(Key, Nonce));
/// // encrypt one block
/// cipher.Transform(Input, 0, Output, 0);
/// </code>
/// </example>
///
/// <example>
/// <description>Decrypting using multi-threading and CBC-WBV:</description>
/// <code>
/// CBC cipher(new AHX());
/// // enable parallel and set the parallel input block size
/// cipher.IsParallel() = true;
/// // calculated automatically based on cache size, but overridable
/// cipher.ParallelBlockSize() = cipher.ProcessorCount() * 32000;
/// // initialize for decryption
/// cipher.Initialize(false, SymmetricKey(Key, Nonce));
/// // decrypt one parallel sized block
/// cipher.Transform128(Input, 0, Output, 0);
/// </code>
/// </example>
/// 
/// <remarks>
/// <description><B>Overview:</B></description>
/// <para>The Cipher Block Chaining cipher mode wraps a symmetric block cipher, enabling the processing of multiple contiguous input blocks to produce a unique cipher-text output. \n
/// The mechanism used in CBC mode can be described as XOR chaining of message input blocks; the first block is XOR'd with the initialization vector, then encrypted with the underlying symmetric cipher.
/// The second block is XOR'd with the first encrypted block, then encrypted, and all subsequent blocks follow this pattern. \n
/// The decryption function follows the reverse pattern; the block is decrypted with the symmetric cipher, and then XOR'd with the ciphertext from the previous block to produce the plain-text.</para>
/// 
/// <description><B>Description:</B></description>
/// <para><EM>Legend:</EM> \n 
/// <B>C</B>=ciphertext, <B>P</B>=plaintext, <B>K</B>=key, <B>E</B>=encrypt, <B>D</B>=decrypt, <B>^</B>=XOR \n
/// <EM>Encryption</EM> \n
/// C0 ← IV. For 1 ≤ j ≤ t, Cj ← EK(Cj−1 ^ Pj). \n
/// <EM>Decryption</EM> \n
/// C0 ← IV. For 1 ≤ j ≤ t, Pj ← Cj−1 ^ E<SUP>−1</SUP>K(Cj).</para>
///
/// <description><B>Multi-Threading:</B></description>
/// <para>The encryption function of the CBC mode is limited by its dependency chain; that is, each block relies on information from the previous block, and so can not be multi-threaded.
/// The decryption function however, is not limited by this dependency chain and can be parallelized via the use of simultaneous processing by multiple processor cores. \n
/// This is achieved by storing the starting vector, (the encrypted bytes), from offsets within the ciphertext stream, and then processing multiple blocks of cipher-text independently across threads. \n 
/// The CBC parallel decryption mode also leverages SIMD instructions to 'double parallelize' those segments. A block of cipher-text assigned to a thread
/// uses SIMD instructions to decrypt 4 or 8 blocks in parallel per cycle, depending on which framework is runtime available, 128 or 256 SIMD instructions.</para>
///
/// <description><B>CBC-WBV:</B></description>
/// <para>Wide Block Vectorization is an extension of the standard CBC mode. Instead of processing a single 16 byte block of input, WBV processes 4 or 8 blocks concurrently using SIMD instructions. \n
/// The underlying block cipher contains the functions Transform64() and Transform128(), which use parallel instructions (SSE3 or AVX dedending on runtime availability), to process multiple input blocks simultaneously. \n
/// This has two adavantages; the first being that if the longer initialization vector is secure (64 or 128 bytes), there is a corresponding increase in security. The second advantage is performance. \n
/// Even if a mode is limited by dependency chaining, like the encryption function of the CBC mode, it can still be parallelized using this method, processing input several times faster than the standard 
/// sequential mode configuration. \n
/// Just as with the standard block size, the decryption function is multi-threaded, maximizing the potential throughput of this extended mode.</para>
///
/// <description><B>Implementation Notes:</B></description>
/// <list type="bullet">
/// <item><description>A cipher mode constructor can either be initialized with a block cipher instance, or using the block ciphers enumeration name.</description></item>
/// <item><description>A block cipher instance created using the enumeration constructor, is automatically deleted when the class is destroyed.</description></item>
/// <item><description>The Transform functions are virtual, and can be accessed from an ICipherMode instance.</description></item>
/// <item><description>The DecryptBlock, Decrypt64, Decrypt128  EncryptBlock, Encrypt64, Encrypt128 functions can be accessed through the class instance.</description></item>
/// <item><description>The transformation methods can not be called until the Initialize(bool, ISymmetricKey) function has been called.</description></item>
/// <item><description>In CBC mode, only the decryption function can be processed in parallel.</description></item>
/// <item><description>The ParallelThreadsMax() property is used as the thread count in the parallel loop; this must be an even number no greater than the number of processer cores on the system.</description></item>
/// <item><description>Parallel processing is enabled on decryption by setting IsParallel() to true, and passing an input block of ParallelBlockSize() to the transform.</description></item>
/// <item><description>ParallelBlockSize() is calculated automatically based on the processor(s) L1 data cache size, this property can be user defined, and must be evenly divisible by ParallelMinimumSize().</description></item>
/// <item><description>Parallel block calculation ex. <c>ParallelBlockSize() = data.size() - (data.size() % cipher.ParallelMinimumSize());</c></description></item>
/// <item><description>CBC-WBV Transforms require cipher initialization with either a 64 or 128 byte Initialization Vector.</description></item>
/// <item><description>CBC-WBV uses the Transform64() or Transform128() functions to process input in 64 or 128 byte message blocks in sequential mode.</description></item>
/// <item><description>CBC-WBV output is <B>not equal</B> to the mode run with a smaller block size; Encryption and Decryption must be performed using an identical block length.</description></item>
/// <item><description>CBC-WBV uses ParallelBlockSize() sized input message blocks to process in multi-threaded mode.</description></item>
/// </list>
/// 
/// <description>Guiding Publications:</description>
/// <list type="number">
/// <item><description>NIST <a href="http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf">SP800-38A</a>.</description></item>
/// <item><description>Handbook of Applied Cryptography <a href="http://cacr.uwaterloo.ca/hac/about/chap7.pdf">Chapter 7: Block Ciphers</a>.</description></item>
/// </list>
/// </remarks>
class CBC : public ICipherMode
{
private:

	static const size_t BLOCK_SIZE = 16;

	IBlockCipher* m_blockCipher;
	size_t m_blockSize;
	std::vector<byte> m_cbcVector;
	BlockCiphers m_cipherType;
	bool m_destroyEngine;
	bool m_isDestroyed;
	bool m_isEncryption;
	bool m_isInitialized;
	bool m_isLoaded;
	ParallelOptions m_parallelProfile;

public:

	CBC(const CBC&) = delete;
	CBC& operator=(const CBC&) = delete;
	CBC& operator=(CBC&&) = delete;

	//~~~Properties~~~//

	/// <summary>
	/// Get: Block size of internal cipher in bytes
	/// </summary>
	virtual const size_t BlockSize() { return m_blockSize; }

	/// <summary>
	/// Get: The block ciphers formal type name
	/// </summary>
	virtual BlockCiphers CipherType() { return m_cipherType; }

	/// <summary>
	/// Get: The underlying Block Cipher instance
	/// </summary>
	virtual IBlockCipher* Engine() { return m_blockCipher; }

	/// <summary>
	/// Get: The Cipher Modes enumeration type name
	/// </summary>
	virtual const CipherModes Enumeral() { return CipherModes::CBC; }

	/// <summary>
	/// Get: True if initialized for encryption, False for decryption
	/// </summary>
	virtual const bool IsEncryption() { return m_isEncryption; }

	/// <summary>
	/// Get: The Block Cipher is ready to transform data
	/// </summary>
	virtual const bool IsInitialized() { return m_isInitialized; }

	/// <summary>
	/// Get: Processor parallelization availability.
	/// <para>Indicates whether parallel processing is available with this mode.
	/// If parallel capable, input/output data arrays passed to the transform must be ParallelBlockSize in bytes to trigger parallelization.</para>
	/// </summary>
	virtual const bool IsParallel() { return m_parallelProfile.IsParallel(); }

	/// <summary>
	/// Get: Array of allowed cipher input key byte-sizes
	/// </summary>
	virtual std::vector<SymmetricKeySize> LegalKeySizes() const { return m_blockCipher->LegalKeySizes(); }

	/// <summary>
	/// Get: The cipher mode name
	/// </summary>
	virtual const std::string Name() { return "CBC"; }

	/// <summary>
	/// Get: The CBC initialization vector (exposed for CMAC)
	/// </summary>
	std::vector<byte> &Nonce() { return m_cbcVector; }

	/// <summary>
	/// Get: Parallel block size; the byte-size of the input/output data arrays passed to a transform that trigger parallel processing.
	/// <para>This value can be changed through the ParallelProfile class.<para>
	/// </summary>
	virtual const size_t ParallelBlockSize() { return m_parallelProfile.ParallelBlockSize(); }

	/// <summary>
	/// Get/Set: Parallel and SIMD capability flags and sizes 
	/// <para>The maximum number of threads allocated when using multi-threaded processing can be set with the ParallelMaxDegree() property.
	/// The ParallelBlockSize() property is auto-calculated, but can be changed; the value must be evenly divisible by ParallelMinimumSize().
	/// Changes to these values must be made before the <see cref="Initialize(SymmetricKey)"/> function is called.</para>
	/// </summary>
	virtual ParallelOptions &ParallelProfile() { return m_parallelProfile; }

	//~~~Constructor~~~//

	/// <summary>
	/// Initialize the Cipher Mode using a block cipher type name
	/// </summary>
	///
	/// <param name="CipherType">The formal enumeration name of a block cipher</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if an undefined block cipher type name is used</exception>
	explicit CBC(BlockCiphers CipherType);

	/// <summary>
	/// Initialize the Cipher Mode using a block cipher instance
	/// </summary>
	///
	/// <param name="Cipher">The uninitialized block cipher instance; can not be null</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if a null block cipher is used</exception>
	explicit CBC(IBlockCipher* Cipher);

	/// <summary>
	/// Finalize objects
	/// </summary>
	virtual ~CBC();

	//~~~Public Functions~~~//

	/// <summary>
	/// Decrypt a single block of bytes.
	/// <para>Decrypts one block of bytes beginning at a zero index.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of encrypted bytes</param>
	/// <param name="Output">The output array of decrypted bytes</param>
	void DecryptBlock(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Decrypt a block of bytes with offset parameters.
	/// <para>Decrypts one block of bytes at the designated offsets.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of encrypted bytes</param>
	/// <param name="InOffset">Starting offset within the Input array</param>
	/// <param name="Output">The output array of decrypted bytes</param>
	/// <param name="OutOffset">Starting offset within the Output array</param>
	void DecryptBlock(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Decrypt a single block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Decrypts one block of bytes beginning at a zero index.
	/// Wide Block Vector format, processes 64 bytes (4* 16 byte blocks) in parallel using 128bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 64 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of encrypted bytes</param>
	/// <param name="Output">The output array of decrypted bytes</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 64 bytes in length</exception>
	void Decrypt64(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Decrypt a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Decrypts one block of bytes using the designated offsets.
	/// Wide Block Vector format, processes 64 bytes (4* 16 byte blocks) in parallel using 128bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 64 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of encrypted bytes</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of decrypted bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 64 bytes in length</exception>
	void Decrypt64(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Decrypt a single block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Decrypts one block of bytes beginning at a zero index.
	/// Wide Block Vector format, processes 128 bytes (8* 16 byte blocks) in parallel using 256bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 128 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of encrypted bytes</param>
	/// <param name="Output">The output array of decrypted bytes</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 128 bytes in length</exception>
	void Decrypt128(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Decrypt a single block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Decrypts one block of bytes using the designated offsets.
	/// Wide Block Vector format, processes 128 bytes (8* 16 byte blocks) in parallel using 256bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 128 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of encrypted bytes</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of decrypted bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 128 bytes in length</exception>
	void Decrypt128(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Release all resources associated with the object
	/// </summary>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if state could not be destroyed</exception>
	virtual void Destroy();

	/// <summary>
	/// Encrypt a single block of bytes. 
	/// <para>Encrypts one block of bytes beginning at a zero index.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of plain text bytes</param>
	/// <param name="Output">The output array of encrypted bytes</param>
	void EncryptBlock(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Encrypt a block of bytes using offset parameters. 
	/// <para>Encrypts one block of bytes at the designated offsets.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of plain text bytes</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of encrypted bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	void EncryptBlock(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Encrypt a single block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Encrypts one block of bytes beginning at a zero index.
	/// Wide Block Vector format, processes 64 bytes (4* 16 byte blocks) in parallel using 128bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 64 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of plain text bytes</param>
	/// <param name="Output">The output array of encrypted bytes</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 64 bytes in length</exception>
	void Encrypt64(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Encrypt a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Encrypts one block of bytes using the designated offsets.
	/// Wide Block Vector format, processes 64 bytes (4* 16 byte blocks) in parallel using 128bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 64 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of plain text bytes</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of encrypted bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 64 bytes in length</exception>
	void Encrypt64(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Encrypt a single block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Encrypts one block of bytes beginning at a zero index.
	/// Wide Block Vector format, processes 128 bytes (8* 16 byte blocks) in parallel using 256bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 128 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of plain text bytes</param>
	/// <param name="Output">The output array of encrypted bytes</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 128 bytes in length</exception>
	void Encrypt128(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Encrypt a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Encrypts one block of bytes using the designated offsets.
	/// Wide Block Vector format, processes 128 bytes (8* 16 byte blocks) in parallel using 256bit SIMD instructions.
	/// The Initialization Vector (Nonce) set with Initialize(), must be 128 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of plain text bytes</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of encrypted bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 128 bytes in length</exception>
	void Encrypt128(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Initialize the Cipher instance
	/// </summary>
	/// 
	/// <param name="Encryption">True if cipher is used for encryption, False to decrypt</param>
	/// <param name="KeyParams">SymmetricKey containing the encryption Key and Initialization Vector</param>
	/// 
	/// <exception cref="CryptoCipherModeException">Thrown if a null Key or Nonce is used</exception>
	virtual void Initialize(bool Encryption, ISymmetricKey &KeyParams);

	/// <summary>
	/// Set the maximum number of threads allocated when using multi-threaded processing.
	/// <para>When set to zero, thread count is set automatically. If set to 1, sets IsParallel() to false and runs in sequential mode. 
	/// Thread count must be an even number, and not exceed the number of processor cores.</para>
	/// </summary>
	///
	/// <param name="Degree">The desired number of threads</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if an invalid degree setting is used</exception>
	void ParallelMaxDegree(size_t Degree);

	/// <summary>
	/// Transform an entire block of bytes. 
	/// <para>Encrypts one block of bytes beginning at a zero index.
	/// This method is used in a buffering strategy, where buffers of size BlockSize() for sequential processing, 
	/// or ParallelBlockSize() for parallel processing, are processed and copied to a larger array by the caller.
	/// Parallel processing is limited to the Decryption function only, the Encryption function will process ParallelBlockSize() blocks in sequential mode.
	/// Buffers should be of the same size, either BlockSize() or ParallelBlockSize().
	/// If the Input and Output array sizes differ, the smallest array size will be processed.
	/// To disable parallel processing, set the ParallelOptions().IsParallel() property to false.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of bytes to transform</param>
	/// <param name="Output">The output array of transformed bytes</param>
	/// 
	/// <returns>The number of bytes processed</returns>
	virtual size_t Transform(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Transform a block of bytes with offset parameters.
	/// <para>Transforms one block of bytes at the designated offsets.
	/// This method is used when looping through two large arrays utilizing offsets incremented by the caller.
	/// One block is processed of either ParallelBlockSize() for parallel processing, or BlockSize() for sequential mode.
	/// Parallel processing is limited to the Decryption function only, the Encryption function will process ParallelBlockSize() blocks in sequential mode.
	/// To disable parallel processing, set the ParallelOptions().IsParallel() property to false.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of bytes to transform</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of transformed bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	/// 
	/// <returns>The number of bytes processed</returns>
	virtual size_t Transform(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Transform a length of bytes with offset parameters. 
	/// <para>This method processes a specified length of bytes, utilizing offsets incremented by the caller.
	/// If IsParallel() is set to true, and the length is at least ParallelBlockSize(), the transform is run in parallel processing mode.
	/// Parallel processing is limited to the Decryption function only, the Encryption function will process ParallelBlockSize() blocks in sequential mode.
	/// To disable parallel processing, set the ParallelOptions().IsParallel() property to false.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	/// 
	/// <param name="Input">The input array of bytes to transform</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of transformed bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	/// <param name="Length">The number of bytes to transform</param>
	virtual void Transform(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset, const size_t Length);

	/// <summary>
	/// Transform a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Transforms one block of bytes beginning at a zero index.
	/// Wide Block Vector format, processes 64 bytes (4* 16 byte blocks) in parallel using 128bit SIMD instructions. 
	/// Multi-threading capable in Decryption mode; set IsParallel() to true to enable, and process blocks of ParallelBlockSize().
	/// The Initialization Vector (Nonce) set with Initialize(), must be 64 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	///
	/// <param name="Input">The input array to transform</param>
	/// <param name="Output">The output array of transformed bytes</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 64 bytes in length</exception>
	void Transform64(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Transform a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Transforms one block of bytes using the designated offsets.
	/// Wide Block Vector format, processes 64 bytes (4* 16 byte blocks) in parallel using 128bit SIMD instructions. 
	/// Multi-threading capable in Decryption mode; set IsParallel() to true to enable, and process blocks of ParallelBlockSize().
	/// The Initialization Vector (Nonce) set with Initialize(), must be 64 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	///
	/// <param name="Input">The input array to transform</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of transformed bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 64 bytes in length</exception>
	void Transform64(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

	/// <summary>
	/// Transform a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Transforms one block of bytes beginning at a zero index.
	/// Wide Block Vector format, processes 128 bytes (8* 16 byte blocks) in parallel using 256bit SIMD instructions.
	/// Multi-threading capable in Decryption mode; set IsParallel() to true to enable, and process blocks of ParallelBlockSize().
	/// The Initialization Vector (Nonce) set with Initialize(), must be 128 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	///
	/// <param name="Input">The input array to transform</param>
	/// <param name="Output">The output array of transformed bytes</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 128 bytes in length</exception>
	void Transform128(const std::vector<byte> &Input, std::vector<byte> &Output);

	/// <summary>
	/// Transform a block of bytes using a Wide Block Vector (CBC-WBV).
	/// <para>Transforms one block of bytes using the designated offsets.
	/// Wide Block Vector format, processes 128 bytes (8* 16 byte blocks) in parallel using 256bit SIMD instructions.
	/// Multi-threading capable in Decryption mode; set IsParallel() to true to enable, and process blocks of ParallelBlockSize().
	/// The Initialization Vector (Nonce) set with Initialize(), must be 128 bytes in length.
	/// Initialize(bool, ISymmetricKey) must be called before this method can be used.</para>
	/// </summary>
	///
	/// <param name="Input">The input array to transform</param>
	/// <param name="InOffset">Starting offset within the input array</param>
	/// <param name="Output">The output array of transformed bytes</param>
	/// <param name="OutOffset">Starting offset within the output array</param>
	///
	/// <exception cref="Exception::CryptoCipherModeException">Thrown if the block size is not 128 bytes in length</exception>
	void Transform128(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);

private:

	void DecryptParallel(const std::vector<byte> &Input, const size_t InOffset, std::vector<byte> &Output, const size_t OutOffset);
	void DecryptSegment(const std::vector<byte> &Input, size_t InOffset, std::vector<byte> &Output, size_t OutOffset, std::vector<byte> &Iv, const size_t BlockCount);
	void Scope();
};

NAMESPACE_MODEEND
#endif
