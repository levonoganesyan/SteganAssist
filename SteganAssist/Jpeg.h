#pragma once
#include<map>
#include<vector>
#include<algorithm>
#include"BitStream.h"
#include"ImageFileBuffer.h"
#include"Image.h"

// [ISO/IEC 10918-1 : 1993(E)]
class Jpeg : public Image
{
	class HuffmanTree
	{
	public:

		struct Node
		{
			Node *_left, *_right;
			byte _value;
			bool _code_end;
			Node();
		};

		class HuffmanTreeIterator
		{
			Node* _state;
			Node* _root;
		public:
			HuffmanTreeIterator(HuffmanTree* tree_);
			HuffmanTreeIterator* Step(int way_);
			bool IsCodeEnd();
			byte GetValue();
			void Reset();
		};
	private:
		
		Node* _root;
		Node* _state;

		bool add_element(Node*& current_node_, int lenght_, int value_);
	public:

		HuffmanTree();
		void AddElement(int lenght_, int value_);
		Node* GetRoot();
	};

	
	struct Frame
	{
		byte _id;
		byte _horizontal_thinning;
		byte _vertical_thinning;
		byte _id_of_quantization_table;
	};

private:
	// Table B.1 – Marker code assignments
	enum markers
	{
		// Start Of Frame markers, non-differential, Huffman coding
		SOF0 = 0xC0, // Baseline DCT
		SOF1 = 0xC1, // Extended sequential DCT
		SOF2 = 0xC2, // Progressive DCT
		SOF3 = 0xC3, // Lossless (sequential)
		
		// Start Of Frame markers, differential, Huffman coding
		SOF5 = 0xC5, // Differential sequential DCT
		SOF6 = 0xC6, // Differential progressive DCT
		SOF7 = 0xC7, // Differential lossless (sequential)

		// Start Of Frame markers, non-differential, arithmetic coding
		JPG = 0xC8, // Reserved for JPEG extensions
		SOF9 = 0xC9, // Extended sequential DCT
		SOF10 = 0xCA, // Progressive DCT
		SOF11 = 0xCB, // Lossless (sequential)

		// Start Of Frame markers, differential, arithmetic coding
		SOF13 = 0xCD, // Differential sequential DCT
		SOF14 = 0xCE, // Differential progressive DCT
		SOF15 = 0xCF, // Differential lossless (sequential)

		// Huffman table specification
		DHT = 0xC4, // Define Huffman Table(s)

		// Arithmetic coding conditioning specification
		DAC = 0xCC, // Define arithmetic coding conditioning(s)

		// Restart interval termination
		RST0 = 0xD0, // Restart with modulo 8 count <0>
		RST1 = 0xD1, // Restart with modulo 8 count <1>
		RST2 = 0xD2, // Restart with modulo 8 count <2>
		RST3 = 0xD3, // Restart with modulo 8 count <3>
		RST4 = 0xD4, // Restart with modulo 8 count <4>
		RST5 = 0xD5, // Restart with modulo 8 count <5>
		RST6 = 0xD6, // Restart with modulo 8 count <6>
		RST7 = 0xD7, // Restart with modulo 8 count <7>

		// Other markers
		SOI = 0xD8, // Start of image
		EOI = 0xD9, // End of image
		SOS = 0xDA, // Start of scan
		DQT = 0xDB, // Define quantization table(s)
		DNL = 0xDC, // Define number of lines
		DRI = 0xDD, // Define restart interval
		DHP = 0xDE, // Define hierarchical progression
		EXP = 0xDF, // Expand reference component(s)
		APP0 = 0xE0, // Reserved for application segments
		APP1 = 0xE1, // Reserved for application segments
		APP2 = 0xE2, // Reserved for application segments
		APP3 = 0xE3, // Reserved for application segments
		APP4 = 0xE4, // Reserved for application segments
		APP5 = 0xE5, // Reserved for application segments
		APP6 = 0xE6, // Reserved for application segments
		APP7 = 0xE7, // Reserved for application segments
		JPG0 = 0xF0, // Reserved for JPEG extensions
		JPG1 = 0xF1, // Reserved for JPEG extensions
		JPG2 = 0xF2, // Reserved for JPEG extensions
		JPG3 = 0xF3, // Reserved for JPEG extensions
		JPG4 = 0xF4, // Reserved for JPEG extensions
		JPG5 = 0xF5, // Reserved for JPEG extensions
		JPG6 = 0xF6, // Reserved for JPEG extensions
		JPG7 = 0xF7, // Reserved for JPEG extensions
		COM = 0xFE, // Comment

		// Reserved markers
		TEM = 0x01, // For temporary private use in arithmetic coding
	};
	
	enum coef_type 
	{ 
		// [3.1.35]
		DC = 0,
		// [3.1.2]
		AC = 1,
	};

	bool check_for_image_correctness(InputBitStream& image_content_);
	// void process_start_of_image(InputBitStream& image_content_);

	/**
	 * \verbatim
	 * [B.2.2] Frame header syntax
	 * Figure 1: frame header
	 * _________________________________________________________
	 * |    |   |   |   |   |   |   |   |   |    | Comp.-spec. |
	 * |  SOFn  |  Lf   | P |   Y   |   X   | Nf | parameters  |
	 * |____|___|___|___|___|___|___|___|___|____|_____________|
	 *
	 *
	 * Figure 2: frame component-specification parameters
	 * ____________________________________________________
	 * |   |     |    |   |     |    |     |   |     |    |
	 * | C | H|V | Tq | C | H|V | Tq | ... | C | H|V | Tq |
	 * |___|_____|____|___|_____|____|_____|___|_____|____|
	 * |______________|______________|     |______________|
	 *         |              |                    |
	 *         v              v                    v
	 *         1              2                   Nf
	 * \endverbatim
	 *
	 *
	 *
	 * * * * Description for Figure 1 * * * *
	 *
	 * SOFn: Start of frame marker:
	 * > Marks the beginning of the frame parameters.
	 * * The subscript n identifies whether the encoding process is baseline
	 * * sequential, extended sequential, progressive, or lossless, as well
	 * * as which entropy encoding procedure is used.
	 * * SOF0: Baseline DCT
	 * * SOF1: Extended sequential DCT, Huffman coding
	 * * SOF2: Progressive DCT, Huffman coding
	 * * SOF3: Lossless (sequential), Huffman coding
	 * * SOF9: Extended sequential DCT, arithmetic coding
	 * * SOF10: Progressive DCT, arithmetic coding
	 * * SOF11: Lossless (sequential), arithmetic coding
	 *
	 * Lf: Frame header length:
	 * > Specifies the length of the frame header shown in Figure 1.
	 *
	 * P: Sample precision:
	 * > Specifies the precision in bits for the samples of the components in the frame.
	 *
	 * Y: Number of lines:
	 * > Specifies the maximum number of lines in the source image.
	 * * This shall be equal to the number of lines in the component with the maximum
	 * * number of vertical samples. Value 0 indicates that the number of lines shall
	 * * be defined by the DNL marker and parameters at the end of the first scan.
	 *
	 * X: Number of samples per line:
	 * > Specifies the maximum number of samples per line in the source image.
	 * * This shall be equal to the number of samples per line in the component with
	 * * the maximum number of horizontal samples
	 *
	 * Nf: Number of image components in frame:
	 * > Specifies the number of source image components in the frame.
	 * * The value of Nf shall be equal to the number of sets of frame component
	 * * specification parameters (Ci, Hi, Vi, and Tqi) present in the frame header.
	 *
	 *
	 * * * * Description for Figure 2 * * * *
	 *
	 * Ci: Component identifier:
	 * > Assigns a unique label to the ith component in the sequence of frame
	 * > component specification parameters.
	 * * These values shall be used in the scan headers to identify the components in
	 * * the scan. The value of Ci shall be different from the values of C1 through Ci - 1.
	 *
	 * Hi: Horizontal sampling factor:
	 * > Specifies the relationship between the component horizontal dimension and
	 * > maximum image dimension X. Also specifies the number of horizontal data units
	 * > of component Ci in each MCU, when more than one component is encoded in a scan.
	 *
	 * Vi: Vertical sampling factor:
	 * > Specifies the relationship between the component vertical dimension and maximum
	 * > image dimension Y. Also specifies the number of vertical data units of component
	 * > Ci in each MCU, when more than one component is encoded in a scan.
	 *
	 * Tqi: Quantization table destination selector:
	 * > Specifies one of four possible quantization table destinations from which the
	 * > quantization table to use for dequantization of DCT coefficients of component Ci
	 * > is retrieved. 
	 * * If the decoding process uses the dequantization procedure, this table
	 * * shall have been installed in this destination by the time the decoder is ready
	 * * to decode the scans containing component Ci. The destination shall not be respecified,
	 * * or its contents changed, until all scans containing Ci have been completed.
	 *
	 * \verbatim
	 * Figure 3: frame header parameter sizes and values
	 * _____________________________________________________________________________
	 * |           |            |                                                  |
	 * |           |            |                      Values                      |
	 * |           |            |__________________________________________________|
	 * |           |            |                     |                 |          |
	 * | Parameter | Size(bits) |    Sequential DCT   |                 |          |
	 * |           |            |_____________________|                 |          |
	 * |           |            |          |          | Progressive DCT | Lossless |
	 * |           |            | Baseline | Extended |                 |          |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |                                                  |
	 * |    Lf     |     16     |                   8 + 3 * Nf                     |
	 * |___________|____________|__________________________________________________|
	 * |           |            |          |          |                 |          |
	 * |     P     |      8     |     8    |   8, 12  |      8, 12      |   2-16   |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |                                                  |
	 * |     Y     |     16     |                     0-65535                      |
	 * |___________|____________|__________________________________________________|
	 * |           |            |                                                  |
	 * |     X     |     16     |                     1-65535                      |
	 * |___________|____________|__________________________________________________|
	 * |           |            |          |          |                 |          |
	 * |    Nf     |      8     |  1-255   |  1-255   |       1-4       |  1-255   |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |                                                  |
	 * |    Ci     |      8     |                      0-255                       |
	 * |___________|____________|__________________________________________________|
	 * |           |            |                                                  |
	 * |    Hi     |      4     |                       1-4                        |
	 * |___________|____________|__________________________________________________|
	 * |           |            |                                                  |
	 * |    Vi     |      4     |                       1-4                        |
	 * |___________|____________|__________________________________________________|
	 * |           |            |          |          |                 |          |
	 * |    Tqi    |      8     |   0-3    |   0-3    |       0-3       |    0     |
	 * |___________|____________|__________|__________|_________________|__________|
	 * \endverbatim
	 *
	 */
	void process_start_of_frame_baseline_DCT(InputBitStream& image_content_);
	void process_start_of_frame_extended_sequential_DCT(InputBitStream& image_content_);
	void process_start_of_frame_progressive_DCT(InputBitStream& image_content_);
	
	/**
	* \verbatim
	* [B.2.4.2] Huffman table-specification syntax
	*
	* Figure 1: define Huffman table segment
	* ___________________________________________________________________________
	* |    |    |    |    |       |      |      |       |       | Symbol-length |
	* |   DHT   |    Lh   | Tc|Th |  Q1  |  Q2  |  ...  |  Q16  |  assignment   |
	* |____|____|____|____|_______|______|______|_______|_______|_______________|
	*
	* Figure 2: symbol-length assignment parameters
	* _______________________________________________________________________________________________________
	* |      |      |       |       |      |      |       |       |       |       |       |       |         |
	* | v1,1 | v1,2 |  ...  | v1,L1 | v2,1 | v2,2 |  ...  | v2,L2 |  ...  | v16,1 | v16,2 |  ...  | v16,L16 |
	* |______|______|_______|_______|______|______|_______|_______|_______|_______|_______|_______|_________|
	*
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* DHT: Define Huffman table marker:
	* > Marks the beginning of Huffman table definition parameters.
	*
	* Lh: Huffman table definition length:
	* > Specifies the length of all Huffman table parameters shown in Figure 1.
	*
	* Tc: Table class:
	* > 0 = DC table or lossless table, 1 = AC table.
	*
	* Th: Huffman table destination identifier:
	* > Specifies one of four possible destinations at the decoder into which
	* > the Huffman table shall be installed.
	*
	* Li: Number of Huffman codes of length i:
	* > Specifies the number of Huffman codes for each of the 16 possible lengths
	* > allowed by this Specification. Li’s are the elements of the list BITS.
	*
	* Vi,j: Value associated with each Huffman code:
	* > Specifies, for each i, the value associated with each Huffman code of
	* > length i. The meaning of each value is determined by the Huffman coding
	* > model.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                                   |
	* |    Lh     |     16     |      2 + sum{t, 1, n}(17 + sum{i, 1, 16}(Li))     |
	* |___________|____________|___________________________________________________|
	* |           |            |                                       |           |
	* |    Tc     |      4     |                   0,1                 |     0     |
	* |___________|____________|_______________________________________|___________|
	* |           |            |          |                                        |
	* |    Th     |      4     |    0,1   |                    0-3                 |
	* |___________|____________|__________|________________________________________|
	* |           |            |                                                   |
	* |    Li     |      8     |                        1-255                      |
	* |___________|____________|___________________________________________________|
	* |           |            |                                                   |
	* |   Vi,j    |      8     |                        1-255                      |
	* |___________|____________|___________________________________________________|
	*
	* \endverbatim
	*
	*/
	void process_huffman_table(InputBitStream& image_content_);

	/**
	* \verbatim
	* [B.2.4.1] Quantization table-specification syntax
	*
	* Figure 1: quantization table syntax
	* __________________________________________________________
	* |    |    |    |    |       |      |      |       |       |
	* |   DQT   |    Lq   | Pq|Tq |  Q0  |  Q1  |  ...  |  Q63  |
	* |____|____|____|____|_______|______|______|_______|_______|
	*                     |_____________________________________|
	*                                         |
	*                                         v
	*                              Multiple(t = 1, ..., n)
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* DQT: Define quantization table marker:
	* > Marks the beginning of quantization table-specification parameters.
	*
	* Lq: Quantization table definition length:
	* > Specifies the length of all quantization table parameters shown in Figure 1.
	*
	* Pq: Quantization table element precision:
	* > Specifies the precision of the Qk values.
	* * Value 0 indicates 8-bit Qk values; value 1 indicates 16-bit Qk values.
	* * Pq shall be zero for 8 bit sample precision P (see B.2.2).
	*
	* Tq: Quantization table destination identifier:
	* > Specifies one of four possible destinations at the decoder into which the
	* > quantization table shall be installed.
	*
	* Qk: Quantization table element:
	* > Specifies the kth element out of 64 elements, where k is the index in the zigzag
	* > ordering of the DCT coefficients.
	* * The quantization elements shall be specified in zig-zag scan order.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                       |           |
	* |    Lq     |     16     |    2 + sum{t, 1, n}(65 + 64*Pq(t))    | Undefined |
	* |___________|____________|_______________________________________|___________|
	* |           |            |          |          |                 |           |
	* |    Pq     |      4     |     0    |   0, 1   |      0, 1       | Undefined |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                       |           |
	* |    Tq     |      4     |                   0-3                 | Undefined |
	* |___________|____________|_______________________________________|___________|
	* |           |            |                                       |           |
	* |    Qk     |    8, 16   |              1-255, 1-65535           | Undefined |
	* |___________|____________|_______________________________________|___________|
	*
	* \endverbatim
	*
	*/
	void process_quantization_table(InputBitStream& image_content_);

	/**
	* \verbatim
	* [B.2.4.3] Arithmetic conditioning table-specification syntax
	*
	* Figure 1: define arithmetic conditioning segment
	* ____________________________________
	* |    |    |    |    |       |      |
	* |   DAC   |    La   | Tc|Tb |  Cs  |
	* |____|____|____|____|_______|______|
	*
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* DAC: Define arithmetic coding conditioning marker:
	* > Marks the beginning of the definition of arithmetic coding conditioning
	* > parameters.
	*
	* La: Arithmetic coding conditioning definition length:
	* > Specifies the length of all arithmetic coding conditioning parameters shown
	* > in Figure 1.
	*
	* Tc: Table class:
	* > 0 = DC table or lossless table, 1 = AC table.
	*
	* Tb: Arithmetic coding conditioning table destination identifier:
	* > Specifies one of four possible destinations at the decoder into which the
	* > arithmetic coding conditioning table shall be installed.
	*
	* Cs: Conditioning table value:
	* > Value in either the AC or the DC (and lossless) conditioning table.
	* * A single value of Cs shall follow each value of Tb. For AC conditioning
	* * tables Tc shall be one and Cs shall contain a value of Kx in the range
	* * 1 <= Kx <= 63. For DC (and lossless) conditioning tables Tc shall be zero
	* * and Cs shall contain two 4-bit parameters, U and L. U and L shall be in the
	* * range 0 <= L <= U <= 15 and the value of Cs shall be L + 16 * U.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |          |                                        |
	* |    La     |     16     |   Undef  |               2 + 2 * n                |
	* |___________|____________|__________|________________________________________|
	* |           |            |          |                            |           |
	* |    Tc     |      4     |   Undef  |              0,1           |     0     |
	* |___________|____________|__________|____________________________|___________|
	* |           |            |          |                                        |
	* |    Tb     |      4     |   Undef  |                    0-3                 |
	* |___________|____________|__________|________________________________________|
	* |           |            |          |                            |           |
	* |    Cs     |      8     |   Undef  |   0-255(Tc=0), 1-63(Tc=1)  |   1-255   |
	* |___________|____________|__________|____________________________|___________|
	*
	* \endverbatim
	*
	*/
	void process_arithmetic_table(InputBitStream& image_content_);

	/**
	 * \verbatim
	 * [B.2.3] Scan header syntax
	 * Figure 1: scan header
	 * ___________________________________________________________
	 * |    |    |    |    |    |  Comp.-spec. |    |    |       |
	 * |   SOS   |    Ls   | Ns |  parameters  | Ss | Se | Ah|Al |
	 * |____|____|____|____|____|______________|____|____|_______|
	 *
	 * Figure 2: scan component-specification parameters
	 * ______________________________________________
	 * |    |       |    |       |     |    |       |
	 * | Cs | Td|Ta | Cs | Td|Ta | ... | Cs | Td|Ta |
	 * |____|_______|____|_______|_____|____|_______|
	 * |____________|____________|     |____________|
	 *        |            |                  |
	 *        v            v                  v
	 *        1            2                  Ns
	 * \endverbatim
	 *
	 * * * * Description for Figure 1 * * * *
	 *
	 * SOS: Start of scan marker:
	 * > Marks the beginning of the scan parameters.
	 *
	 * Ls: Scan header length:
	 * > Specifies the length of the scan header shown in Figure 1.
	 *
	 * Ns: Number of image components in scan:
	 * > Specifies the number of source image components in the scan.
	 * * The value of Ns shall be equal to the number of sets of scan component
	 * * specification parameters (Csj, Tdj, and Taj) present in the scan header.
	 *
	 * Ss: Start of spectral or predictor selection:
	 * > In the DCT modes of operation, this parameter specifies the first DCT coefficient
	 * > in each block in zig-zag order which shall be coded in the scan.
	 * * This parameter shall be set to zero for the sequential DCT processes. In the
	 * * lossless mode of operations this parameter is used to select the predictor.
	 *
	 * Se: End of spectral selection:
	 * > Specifies the last DCT coefficient in each block in zig-zag order which shall be
	 * > coded in the scan.
	 * * This parameter shall be set to 63 for the sequential DCT processes. In the lossless
	 * * mode of operations this parameter has no meaning. It shall be set to zero.
	 *
	 * Ah: Successive approximation bit position high:
	 * > This parameter specifies the point transform used in the preceding scan (i.e.
	 * > successive approximation bit position low in the preceding scan) for the band of
	 * > coefficients specified by Ss and Se.
	 * * This parameter shall be set to zero for the first scan of each band of coefficients.
	 * * In the lossless mode of operations this parameter has no meaning. It shall be set to
	 * * zero.
	 *
	 * Al: Successive approximation bit position low or point transform:
	 * > In the DCT modes of operation this parameter specifies the point transform, i.e.
	 * > bit position low, used before coding the band of coefficients specified by Ss and Se.
	 * * This parameter shall be set to zero for the sequential DCT processes. In the lossless
	 * * mode of operations, this parameter specifies the point transform, Pt.
	 *
	 * * * * Description for Figure 2 * * * *
	 *
	 * Csj: Scan component selector:
	 * > Selects which of the Nf image components specified in the frame parameters
	 * > shall be the jth component in the scan.
	 * * Each Csj shall match one of the Ci values specified in the frame header,
	 * * and the ordering in the scan header shall follow the ordering in the frame
	 * * header. If Ns > 1, the order of interleaved components in the MCU is Cs1
	 * * first, Cs2 second, etc. If Ns > 1, the following restriction shall be placed
	 * * on the image components contained in the scan:
	 * * sum {j, 1, Ns} {Hj * Vj} <= 10
	 * * where Hj and Vj are the horizontal and vertical sampling factors for scan
	 * * component j. These sampling factors are specified in the frame header for
	 * * component i, where i is the frame component specification index for which
	 * * frame component identifier Ci matches scan component selector Csj.
	 * * The value of Csj shall be different from the values of Cs1 to Csj – 1.
	 * *
	 * $ As an example, consider an image having 3 components with maximum dimensions
	 * $ of 512 lines and 512 samples per line, and with the following sampling factors:
	 * $ _________________________________
	 * $ |             |        |        |
	 * $ | Component 0 | H0 = 4 | V0 = 1 |
	 * $ |_____________|________|________|
	 * $ |             |        |        |
	 * $ | Component 1 | H1 = 1 | V1 = 2 |
	 * $ |_____________|________|________|
	 * $ |             |        |        |
	 * $ | Component 2 | H2 = 2 | V2 = 2 |
	 * $ |_____________|________|________|
	 * $
	 * $ Then the summation of Hj * Vj is (4 * 1) + (1 * 2) + (2 * 2) = 10.
	 *
	 * Tdj: DC entropy coding table destination selector:
	 * > Specifies one of four possible DC entropy coding table destinations from which
	 * > the entropy table needed for decoding of the DC coefficients of component Csj
	 * > is retrieved. The DC entropy table shall have been installed in this destination
	 * > by the time the decoder is ready to decode the current scan. This parameter
	 * > specifies the entropy coding table destination for the lossless processes.
	 *
	 * Taj: AC entropy coding table destination selector:
	 * > Specifies one of four possible AC entropy coding table destinations from which
	 * > the entropy table needed for decoding of the AC coefficients of component Csj
	 * > is retrieved. The AC entropy table selected shall have been installed in this
	 * > destination by the time the decoder is ready to decode the current scan. This
	 * > parameter is zero for the lossless processes.
	 *
	 *
	 * \verbatim
	 * Figure 3: scan header parameter size and values
	 * _____________________________________________________________________________
	 * |           |            |                                                  |
	 * |           |            |                      Values                      |
	 * |           |            |__________________________________________________|
	 * |           |            |                     |                 |          |
	 * | Parameter | Size(bits) |    Sequential DCT   |                 |          |
	 * |           |            |_____________________|                 |          |
	 * |           |            |          |          | Progressive DCT | Lossless |
	 * |           |            | Baseline | Extended |                 |          |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |                                                  |
	 * |    Ls     |     16     |                   6 + 2 * Ns                     |
	 * |___________|____________|__________________________________________________|
	 * |           |            |                                                  |
	 * |    Ns     |      8     |                       1-4                        |
	 * |___________|____________|__________________________________________________|
	 * |           |            |                                                  |
	 * |    Csj    |      8     |                     0-255(a)                     |
	 * |___________|____________|__________________________________________________|
	 * |           |            |          |          |                 |          |
	 * |    Tdj    |      4     |    0-1   |    0-3   |       0-3       |    0-3   |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |          |          |                 |          |
	 * |    Taj    |      4     |    0-1   |    0-3   |       0-3       |     0    |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |          |          |                 |          |
	 * |    Ss     |      8     |     0    |     0    |      0-63       |  1-7(b)  |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |          |          |                 |          |
	 * |    Se     |      8     |    63    |    63    |     Ss-63(c)    |     0    |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |          |          |                 |          |
	 * |    Ah     |      4     |     0    |     0    |       0-13      |     0    |
	 * |___________|____________|__________|__________|_________________|__________|
	 * |           |            |          |          |                 |          |
	 * |    Al     |      4     |     0    |     0    |       0-13      |    0-15  |
	 * |___________|____________|__________|__________|_________________|__________|
	 *
	 * (a) Csj shall be a member of the set of Ci specified in the frame header.
	 * (b) 0 for lossless differential frames in the hierarchical mode (see B.3).
	 * (c) 0 if Ss equals zero.
	 * \endverbatim
	 *
	 */
	void process_start_of_scan(InputBitStream& image_content_);

	/**
	* \verbatim
	* [B.2.4.4] Restart interval definition syntax
	*
	* Figure 1: define restart interval segment
	* _______________________________
	* |    |    |    |    |    |    |
	* |   DRI   |    Lr   |    Ri   |
	* |____|____|____|____|____|____|
	*
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* DRI: Define restart interval marker:
	* > Marks the beginning of the parameters which define the restart interval.
	*
	* Lr: Define restart interval segment length:
	* > Specifies the length of the parameters in the DRI segment shown in Figure 1.
	*
	* Ri: Restart interval:
	* > Specifies the number of MCU in the restart interval.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                                   |
	* |    Lr     |     16     |                        4                          |
	* |___________|____________|___________________________________________________|
	* |           |            |                                       |           |
	* |    Ri     |     16     |                 0-65535               |  n * MCUR |
	* |___________|____________|_______________________________________|___________|
	*
	* \endverbatim
	*
	*/
	void process_restart_interval(InputBitStream& image_content_);

	/**
	* \verbatim
	* [B.2.4.6] Application data syntax
	*
	* Figure 1: application data segment
	* _______________________________________________
	* |    |    |    |    |                         |
	* |   APPn  |    Lp   |  Ap1, Ap2, ..., ApLp-2  |
	* |____|____|____|____|_________________________|
	*
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* APPn: Application data marker:
	* > Marks the beginning of an application data segment.
	*
	* Lp: Application data segment length:
	* > Specifies the length of the application data segment shown in Figure 1.
	*
	* Api: Application data byte:
	* > The interpretation is left to the application.
	*
	* The APPn (Application) segments are reserved for application use.
	* Since these segments may be defined differently for different applications,
	* they should be removed when the data are exchanged between application environments.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                                   |
	* |    Lp     |     16     |                      2-65535                      |
	* |___________|____________|___________________________________________________|
	* |           |            |                                                   |
	* |    Api    |      8     |                       0-255                       |
	* |___________|____________|___________________________________________________|
	*
	* \endverbatim
	*
	*/
	void process_application_specific(InputBitStream& image_content_);

	/**
	* \verbatim
	* [B.2.4.5] Comment syntax
	*
	* Figure 1: comment segment
	* _______________________________________________
	* |    |    |    |    |                         |
	* |   COM   |    Lc   |  Cm1, Cm2, ..., CmLc-2  |
	* |____|____|____|____|_________________________|
	*
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* COM: Comment marker:
	* > Marks the beginning of a comment.
	*
	* Lc: Comment segment length:
	* > Specifies the length of the comment segment shown in Figure 1.
	*
	* Cmi: Comment byte:
	* The interpretation is left to the application.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                                   |
	* |    Lc     |     16     |                      2-65535                      |
	* |___________|____________|___________________________________________________|
	* |           |            |                                                   |
	* |    Cmi    |      8     |                       0-255                       |
	* |___________|____________|___________________________________________________|
	*
	* \endverbatim
	*
	*/
	void process_comment(InputBitStream& image_content_);

	/**
	* \verbatim
	* [B.2.5] Define number of lines syntax
	*
	* Figure 1: define number of lines segment
	* ___________________________
	* |    |   |    |    |      |
	* |   DNL  |    Ld   |  NL  |
	* |____|___|____|____|______|
	*
	* \endverbatim
	*
	* * * * Description for Figure 1 * * * *
	*
	* DNL: Define number of lines marker:
	* Marks the beginning of the define number of lines segment.
	*
	* Ld: Define number of lines segment length:
	* Specifies the length of the define number of lines segment shown in Figure 1.
	*
	* NL: Number of lines:
	* Specifies the number of lines in the frame.
	*
	* \verbatim
	* ______________________________________________________________________________
	* |           |            |                                                   |
	* |           |            |                      Values                       |
	* |           |            |___________________________________________________|
	* |           |            |                     |                 |           |
	* | Parameter | Size(bits) |    Sequential DCT   |                 |           |
	* |           |            |_____________________|                 |           |
	* |           |            |          |          | Progressive DCT |  Lossless |
	* |           |            | Baseline | Extended |                 |           |
	* |___________|____________|__________|__________|_________________|___________|
	* |           |            |                                                   |
	* |     Lp    |     16     |                         4                         |
	* |___________|____________|___________________________________________________|
	* |           |            |                                                   |
	* |     NL    |     16     |                      0-65535                      |
	* |___________|____________|___________________________________________________|
	*
	* \endverbatim
	*
	*/
	void process_number_of_lines(InputBitStream& image_content_);

	void calculating_zigzag_order_traversal(int size_of_table_, int size_of_matrix_);
	// void process_end_of_image(InputBitStream& image_content_);

	InputBitStream _image_content;

	std::map<int,std::vector<HuffmanTree*>> _huffman_trees; // trees for DC and AC coefs
	std::string _comment;
	std::vector<std::vector<std::vector<int>>> _quantization_tables;
	std::vector<std::pair<int, int>> _zigzag_order_traversal_indices;
	std::vector<Frame> _frames;
	byte _max_horizontal_thinning;
	byte _max_vertical_thinning;


public:
	/**
	* \verbatim
	* [B.2.1] High-level syntax
	* Figure 1: compressed image data
	* ____________________________________
	* |    |    |              |    |    |
	* |   SOS   |     Frame    |   EOI   |
	* |____|____|______________|____|____|
	*
	* Figure 2: frame
	* ___________________________________________________________________________________
	* |                |              |       |               |         |     |         |
	* | [Tables/misc.] | Frame header | Scan1 | [DNL segment] | [Scan2] | ... | [ScanN] |
	* |________________|______________|_______|_______________|_________|_____|_________|
	*
	*
	* Figure 3: scan
	* _____________________________________________________________________________________________
	* |                |             |       |    |    |      |     |        |    |     |         |
	* | [Tables/misc.] | Scan header | [ECS0 |   RST0  | ECS1 | ... | ECSn-1 |  RSTn-1] |   ECSn  |
	* |________________|_____________|_______|____|____|______|_____|________|____|_____|_________|
	*
	*
	* Figure 4.1: entropy-coded segment (ECS) 0
	* ________________________________________
	* |         |         |       |          |
	* | <MCU 1> | <MCU 1> |  ···  | <MCU Ri> |
	* |_________|_________|_______|__________|
	*
	* Figure 4.2: entropy-coded segment (ECS) N
	* ____________________________________________
	* |         |           |       |            |
	* | <MCU k> | <MCU k+1> |  ···  | <MCU last> |
	* |_________|___________|_______|____________|
	*
	* \endverbatim
	*
	* * * * Description for Figure 3 * * * *
	*
	* SOI: Start of image marker:
	* > Marks the start of a compressed image represented in the interchange format or abbreviated format.
	*
	* EOI: End of image marker:
	* > Marks the end of a compressed image represented in the interchange format or abbreviated format.
	*
	* RSTm: Restart marker:
	* > A conditional marker which is placed between entropy-coded segments only if restart is enabled.
	* * There are 8 unique restart markers (m = 0 - 7) which repeat in sequence from 0 to 7, starting with
	* * zero for each scan, to provide a modulo 8 restart interval count.
	*
	* * * * Description for Figure 4 and 5 * * * *
	*
	* These Figures specifies that each entropy-coded segment is comprised of a sequence of entropycoded
	* MCUs. If restart is enabled and the restart interval is defined to be Ri, each entropy-coded segment
	* except the last one shall contain Ri MCUs. The last one shall contain whatever number of MCUs completes
	* the scan.
	*
	*
	*
	* [B.2.4]
	* Figure 5: Tables or miscellaneous marker segment
	*
	* __________________________________________
	* |          |          |       |          |
	* |  Marker  |  Marker  |       |  Marker  |
	* | segment1 | segment2 |  ···  | segmentn |
	* |__________|__________|_______|__________|
	*
	*
	*                                              __
	*     Quantization table-specification          |
	*                    or                         |
	*        Huffman table-specification            |
	*                    or                         |
	* Arithmetic conditioning table-specification   |
	*                    or                         |==> Marker segment
	*        Restart interval definition            |
	*                    or                         |
	*                  Comment                      |
	*                    or                         |
	*            Application data                  _|
	*
	*/
	Jpeg(const std::string& file_path_)
		: _image_content(ImageFileBuffer(file_path_).Get())
	{
		// check_for_image_correctness(_image_content);

		_quantization_tables.resize(0x10);

		byte temp;
		while ( _image_content >> temp )
		{
			if (temp != 0xFF)
			{
				throw std::exception("There must be 0xFF byte");
			}
			byte marker;
			_image_content >> marker;
				
			switch (marker)
			{
			case SOI:
				/*i += process_start_of_image(_image_content);*/
				break;
			case SOF0:
				process_start_of_frame_baseline_DCT(_image_content);
				break;
			case SOF1:
				process_start_of_frame_extended_sequential_DCT(_image_content);
				break;
			case SOF2:
				process_start_of_frame_progressive_DCT(_image_content);
				break;
			case DHT:
				process_huffman_table(_image_content);
				break;
			case DQT:
				process_quantization_table(_image_content);
				break;
			case DRI:
				process_restart_interval(_image_content);
				break;
			case SOS:
				process_start_of_scan(_image_content);
				break;
			case RST0:
			case RST1:
			case RST2:
			case RST3:
			case RST4:
			case RST5:
			case RST6:
			case RST7:
				_image_content.BytesBack(1); // 1 is for understanding restart type
				process_restart_interval(_image_content); 
				break;
			case APP0:
			case APP1:
			case APP2:
			case APP3:
			case APP4:
			case APP5:
			case APP6:
			case APP7:
				_image_content.BytesBack(1); // 1 is for understanding application type
				process_application_specific(_image_content);
				break;
			case COM:
				process_comment(_image_content);
				break;
			case EOI:
				break;
			//	process_end_of_image(_image_content);
			default:
				throw std::runtime_error("Found not supported yet marker: " + std::to_string(EOI));
			}
		}



		/*tree = new HuffmanTree();
		tree->AddElement(1, 1);
		tree->AddElement(3, 0);
		tree->AddElement(3, 12);
		tree->AddElement(4, 2);
		tree->AddElement(4, 11);
		tree->AddElement(4, 31);
		tree->AddElement(5, 21);
		std::vector<int> vec = { 1, 1, 1, 0 };
		int i;
		for (i = 0; i < vec.size(); i++)
		{
			if (tree->NextState(vec[i])->code_end)
			{
				break;
			}
		}
		i;*/
	}




};
