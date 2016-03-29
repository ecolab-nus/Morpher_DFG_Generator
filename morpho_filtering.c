#include "morpho_filtering.h"

/////////////////////////////////////////////////////////////////////////////////
//-----------------------PRIVATE MEMORY OF CORE 0,1 and 2----------------------//
/////////////////////////////////////////////////////////////////////////////////
//El vector B1 definido en la tecnica se despliega en memoria
const int32_t B1[] = B1_STRUCTURING_ELEMENT;

//														SIZE IN BYTES PER LEAD
//										 				(250 Hz)		(500 Hz)
volatile int32_t bufferEB0[RMS_NLEADS][TAMB0];			//		100				200
volatile int32_t bufferDB0[RMS_NLEADS][TAMB0];			//		100				200
volatile int32_t bufferDBc[RMS_NLEADS][TAMBc];			//		150				300
volatile int32_t bufferEBc[RMS_NLEADS][TAMBc];			//		150				300
volatile int32_t bufferEnt[RMS_NLEADS][TAMBEnt];		//		300				600
volatile int32_t bufferFBc1[RMS_NLEADS][TAMBFB];		//		10				18
volatile int32_t bufferFBc1ed[RMS_NLEADS][TAMBFB];		//		10				18
volatile int32_t bufferFBc1de[RMS_NLEADS][TAMBFB];		//		10				18
volatile int32_t bufferEntrada[RMS_NLEADS][BUFFER_SIZE_FILTER];//2				2
volatile int32_t bufferSalida[RMS_NLEADS][BUFFER_SIZE_FILTER];//2				2

volatile int32_t samplesRead[RMS_NLEADS];				//		2				2	
volatile int32_t actEntrada[RMS_NLEADS];				//		2				2
volatile int32_t contador[RMS_NLEADS];					//		2				2
volatile int32_t aBEB0[RMS_NLEADS];						//		2				2
volatile int32_t pMinEB0[RMS_NLEADS];					//		2				2
volatile int32_t p2MinEB0[RMS_NLEADS];					//		2				2
volatile int32_t aBDB0[RMS_NLEADS];						//		2				2
volatile int32_t pMaxDB0[RMS_NLEADS];					//		2				2
volatile int32_t p2MaxDB0[RMS_NLEADS];					//		2				2
volatile int32_t aBDBc[RMS_NLEADS];						//		2				2
volatile int32_t pMaxDBc[RMS_NLEADS];					//		2				2
volatile int32_t p2MaxDBc[RMS_NLEADS];					//		2				2
volatile int32_t aBEBc[RMS_NLEADS];						//		2				2
volatile int32_t pMinEBc[RMS_NLEADS];					//		2				2
volatile int32_t p2MinEBc[RMS_NLEADS];					//		2				2
volatile int32_t aBEnt[RMS_NLEADS];						//		2				2
volatile int32_t rBEnt[RMS_NLEADS];						//		2				2
volatile int32_t aBFBc1[RMS_NLEADS];					//		2				2
volatile int32_t aFBc1x[RMS_NLEADS];					//		2				2
volatile int32_t pMaxFBc1ed[RMS_NLEADS];				//		2				2
volatile int32_t pMinFBc1de[RMS_NLEADS];				//		2				2
volatile int32_t p2MinFBc1de[RMS_NLEADS];				//		2				2
volatile int32_t p2MaxFBc1ed[RMS_NLEADS];				//		2				2
volatile int32_t indiceB1[RMS_NLEADS];					//		2				2
volatile int32_t min1E[RMS_NLEADS];						//		2				2
volatile int32_t max1D[RMS_NLEADS];						//		2				2
volatile int32_t vaux[RMS_NLEADS];						//		2				2
volatile int32_t v2aux[RMS_NLEADS];						//		2				2

//-----------------------------------------TOTAL			890				1714

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void storeOutput(int32_t v, int32_t lead) {
	bufferSalida[lead][0] = v;
}

//Opening: dilation(erosion(X))
//Closing: erosion(dilation(X))

//Definicion de buffers segun su aparicion en la tecnica:
//bufferEB0:	opening(entrada) -> erosion(entrada, B0)
//bufferDB0[lead]:	opening(entrada) -> dilation(erosion(entrada, B0))
//bufferDBc[lead]:	closing(fbt)     -> dilation(fbt, Bc)
//bufferEBc[lead]:	closing(fbt)     -> erosion(dilation(fbt, Bc))
//bufferEnt:    buffer necesario para superar la diferencia de tamaÃ±o entre
//              los buffers del final del calculo y los iniciales y poder
//				realizar la resta fbc = entrada - fb
//BufferFBc1:	buffer con los valores fbc para los siguientes
//BufferFBc1e:  opening(fbc)	 -> erosion(fbc, B1);
//BufferFBc1ed: opening(fbc)	 -> dilation(erosion(fbc, B1));
//BufferFBc1d:  closing(fbc)	 -> dilation(fbc, B1);
//BufferFBc1de: closing(fbc)	 -> erosion(dilation(fbc,B1));

//A partir de los valores de BufferFB2de y BufferFB2ed es posible obtener
//el valor resultado del filtrado.


void proceso(int32_t lead) {

 	int32_t i, j;
	int32_t component;

	while (actEntrada[lead] < samplesRead[lead]) {

		if (aBEB0[lead] == pMinEB0[lead]) {
			if (bufferEntrada[lead][actEntrada[lead]] > bufferEB0[lead][pMinEB0[lead]]) {
				if (p2MinEB0[lead] == -1) {
					bufferEB0[lead][aBEB0[lead]] = bufferEntrada[lead][actEntrada[lead]];

					v2aux[lead] = INT16_MAX;
					vaux[lead] = bufferEB0[lead][TAMB0 - 1];
					pMinEB0[lead] = TAMB0 - 1;
					for (i = TAMB0 - 2; i >= 0; i--) {
						if (bufferEB0[lead][i] < vaux[lead]) {
							v2aux[lead] = vaux[lead];
							vaux[lead] = bufferEB0[lead][i];
							p2MinEB0[lead] = pMinEB0[lead];
							pMinEB0[lead] = i;
						} else if (bufferEB0[lead][i] < v2aux[lead]) {
							v2aux[lead] = bufferEB0[lead][i];
							p2MinEB0[lead] = i;
						}
					}
				} else if (bufferEntrada[lead][actEntrada[lead]] > bufferEB0[lead][p2MinEB0[lead]]) {
					pMinEB0[lead] = p2MinEB0[lead];
					p2MinEB0[lead] = -1;
				}
			}
		} else if (bufferEntrada[lead][actEntrada[lead]] <= bufferEB0[lead][pMinEB0[lead]]) {
			p2MinEB0[lead] = pMinEB0[lead];
			pMinEB0[lead] = aBEB0[lead];
		} else if (aBEB0[lead] == p2MinEB0[lead]) {
			if (bufferEntrada[lead][actEntrada[lead]] > bufferEB0[lead][p2MinEB0[lead]]) {
				p2MinEB0[lead] = -1;
			}
		} else if (p2MinEB0[lead] != -1 && bufferEntrada[lead][actEntrada[lead]] <= bufferEB0[lead][p2MinEB0[lead]])
			p2MinEB0[lead] = aBEB0[lead];

		bufferEnt[lead][aBEnt[lead]] = bufferEntrada[lead][actEntrada[lead]];

		if (aBEnt[lead] != TAMBEnt - 1)
			(aBEnt[lead])++;
		else
			aBEnt[lead] = 0;

		bufferEB0[lead][aBEB0[lead]] = bufferEntrada[lead][(actEntrada[lead])++];

		if (aBEB0[lead] != TAMB0 - 1)
			(aBEB0[lead])++;
		else
			(aBEB0[lead]) = 0;

		if (aBDB0[lead] == pMaxDB0[lead]) {
			if (bufferEB0[lead][pMinEB0[lead]] < bufferDB0[lead][pMaxDB0[lead]]) {
				if (p2MaxDB0[lead] == -1) {
					bufferDB0[lead][aBDB0[lead]] = bufferEB0[lead][pMinEB0[lead]];

					v2aux[lead] = INT16_MIN;
					vaux[lead] = bufferDB0[lead][TAMB0 - 1];
					pMaxDB0[lead] = TAMB0 - 1;
					for (i = TAMB0 - 2; i >= 0; i--) {
						if (bufferDB0[lead][i] > vaux[lead]) {
							v2aux[lead] = vaux[lead];
							vaux[lead] = bufferDB0[lead][i];
							p2MaxDB0[lead] = pMaxDB0[lead];
							pMaxDB0[lead] = i;
						} else if (bufferDB0[lead][i] > v2aux[lead]) {
							v2aux[lead] = bufferDB0[lead][i];
							p2MaxDB0[lead] = i;
						}
					}
				} else if (bufferEB0[lead][pMinEB0[lead]] < bufferDB0[lead][p2MaxDB0[lead]]) {
					pMaxDB0[lead] = p2MaxDB0[lead];
					p2MaxDB0[lead] = -1;
				}
			}
		} else if (bufferEB0[lead][pMinEB0[lead]] >= bufferDB0[lead][pMaxDB0[lead]]) {
			p2MaxDB0[lead] = pMaxDB0[lead];
			pMaxDB0[lead] = aBDB0[lead];
		} else if (aBDB0[lead] == p2MaxDB0[lead]) {
			if (bufferEB0[lead][pMinEB0[lead]] < bufferDB0[lead][p2MaxDB0[lead]]) {
				p2MaxDB0[lead] = -1;
			}
		} else if (p2MaxDB0[lead] != -1 && bufferEB0[lead][pMinEB0[lead]] >= bufferDB0[lead][p2MaxDB0[lead]])
			p2MaxDB0[lead] = aBDB0[lead];

		bufferDB0[lead][aBDB0[lead]] = bufferEB0[lead][pMinEB0[lead]];

		if (aBDB0[lead] != TAMB0 - 1){
			(aBDB0[lead])++;
		}
		else{
			aBDB0[lead] = 0;
		}

		//Paso de datos al buffer DBc si se ha llenado el buffer DB0.
	//			if (!firstPass[k] || actEntrada[lead][k] >= TAMB0 + (TAMB0/2) - 1) { // 74
		//VICTOR: he cambiado esto
		if (contador[lead] >= TAMB0 + (TAMB0/2) - 1) {
			if (aBDBc[lead] == pMaxDBc[lead]) {
				if (bufferDB0[lead][pMaxDB0[lead]] < bufferDBc[lead][pMaxDBc[lead]]) {
					if (p2MaxDBc[lead] == -1) {
						bufferDBc[lead][aBDBc[lead]] = bufferDB0[lead][pMaxDB0[lead]];

						v2aux[lead] = INT16_MIN;
						vaux[lead] = bufferDBc[lead][TAMBc - 1];
						pMaxDBc[lead] = TAMBc - 1;
						for (i = TAMBc - 2; i >= 0; i--) {
							if (bufferDBc[lead][i] > vaux[lead]) {
								v2aux[lead] = vaux[lead];
								vaux[lead] = bufferDBc[lead][i];
								p2MaxDBc[lead] = pMaxDBc[lead];
								pMaxDBc[lead] = i;
							} else if (bufferDBc[lead][i] > v2aux[lead]) {
								v2aux[lead] = bufferDBc[lead][i];
								p2MaxDBc[lead] = i;
							}
						}
					} else if (bufferDB0[lead][pMaxDB0[lead]] < bufferDBc[lead][p2MaxDBc[lead]]) {
						pMaxDBc[lead] = p2MaxDBc[lead];
						p2MaxDBc[lead] = -1;
					}
				}
			} else if (bufferDB0[lead][pMaxDB0[lead]] >= bufferDBc[lead][pMaxDBc[lead]]) {
				p2MaxDBc[lead] = pMaxDBc[lead];
				pMaxDBc[lead] = aBDBc[lead];
			} else if (aBDBc[lead] == p2MaxDBc[lead]) {
				if (bufferDB0[lead][pMaxDB0[lead]] < bufferDBc[lead][p2MaxDBc[lead]]) {
					p2MaxDBc[lead] = -1;
				}
			} else if (p2MaxDBc[lead] != -1 && bufferDB0[lead][pMaxDB0[lead]] >= bufferDBc[lead][p2MaxDBc[lead]])
				p2MaxDBc[lead] = aBDBc[lead];

			bufferDBc[lead][aBDBc[lead]] = bufferDB0[lead][pMaxDB0[lead]];

			if (aBDBc[lead] != TAMBc - 1)
				(aBDBc[lead])++;
			else
				aBDBc[lead] = 0;

			//Paso de datos al buffer EBc si se ha llenado DBc
			//if (!firstPass[k] || actEntrada[lead][k] >= (TAMB0*2) + (TAMB0/2) - 2) { //123
			//VICTOR: he cambiado esto.
			if (contador[lead] >= (TAMB0*2) + (TAMB0/2) - 2) {
				if (aBEBc[lead] == pMinEBc[lead]) {
					if (bufferDBc[lead][pMaxDBc[lead]] > bufferEBc[lead][pMinEBc[lead]]) {
						if (p2MinEBc[lead] == -1) {
							bufferEBc[lead][aBEBc[lead]] = bufferDBc[lead][pMaxDBc[lead]];

							v2aux[lead] = INT16_MAX;
							vaux[lead] = bufferEBc[lead][TAMBc - 1];
							pMinEBc[lead] = TAMBc - 1;
							for (i = TAMBc - 2; i >= 0; i--) {
								if (bufferEBc[lead][i] < vaux[lead]) {
									v2aux[lead] = vaux[lead];
									vaux[lead] = bufferEBc[lead][i];
									p2MinEBc[lead] = pMinEBc[lead];
									pMinEBc[lead] = i;
								} else if (bufferEBc[lead][i] < v2aux[lead]) {
									v2aux[lead] = bufferEBc[lead][i];
									p2MinEBc[lead] = i;
								}
							}
						} else if (bufferDBc[lead][pMaxDBc[lead]] > bufferEBc[lead][p2MinEBc[lead]]) {
							pMinEBc[lead] = p2MinEBc[lead];
							p2MinEBc[lead] = -1;
						}
					}
				} else if (bufferDBc[lead][pMaxDBc[lead]] <= bufferEBc[lead][pMinEBc[lead]]) {
					p2MinEBc[lead] = pMinEBc[lead];
					pMinEBc[lead] = aBEBc[lead];
				} else if (aBEBc[lead] == p2MinEBc[lead]) {
					if (bufferDBc[lead][pMaxDBc[lead]] > bufferEBc[lead][p2MinEBc[lead]]) {
						p2MinEBc[lead] = -1;
					}
				} else if (p2MinEBc[lead] != -1 && bufferDBc[lead][pMaxDBc[lead]] <= bufferEBc[lead][p2MinEBc[lead]])
					p2MinEBc[lead] = aBEBc[lead];

				bufferEBc[lead][aBEBc[lead]] = bufferDBc[lead][pMaxDBc[lead]];

				if (aBEBc[lead] != TAMBc - 1)
					(aBEBc[lead])++;
				else
					aBEBc[lead] = 0;

				//Paso de datos al buffer FBc1 si se ha llenado EBc
				//if (!firstPass[k] || actEntrada[lead][k] >= (TAMB0*3) + (TAMB0/2) - (TAMBcm - TAMB0m) - 3) {
				//VICTOR: he cambiado esto.
				if (contador[lead] >= (TAMB0*3) + (TAMB0/2) - (TAMBcm - TAMB0m) - 3) {
					bufferFBc1[lead][aBFBc1[lead]] = bufferEnt[lead][rBEnt[lead]] - bufferEBc[lead][pMinEBc[lead]];

					if (aBFBc1[lead] != TAMBFB - 1)
						(aBFBc1[lead])++;
					else
						aBFBc1[lead] = 0;

					if (rBEnt[lead] != TAMBEnt - 1)
						(rBEnt[lead])++;
					else
						rBEnt[lead] = 0;

					j = indiceB1[lead];
					if (indiceB1[lead] != TAMB1*(TAMB1-1))
						indiceB1[lead] += TAMB1;
					else
						indiceB1[lead] = 0;

					min1E[lead] = bufferFBc1[lead][0] - B1[j];
					max1D[lead] = bufferFBc1[lead][0] + B1[j];

					for(component = 1; component < TAMB1; component++)
					{
						if(min1E[lead]>bufferFBc1[lead][component]-B1[j+component])
						min1E[lead] = bufferFBc1[lead][component] - B1[j + component];
						if(max1D[lead]<bufferFBc1[lead][component]+B1[j+component])
						 	max1D[lead] = bufferFBc1[lead][component] + B1[j + component];
					}

					//Paso de valores y mover puntero de bufferFBc1ed
					if (aFBc1x[lead] == pMaxFBc1ed[lead]) {
						if (min1E[lead] < bufferFBc1ed[lead][pMaxFBc1ed[lead]]) {
							if (p2MaxFBc1ed[lead] == -1) {
								bufferFBc1ed[lead][aFBc1x[lead]] = min1E[lead];

								v2aux[lead] = INT16_MIN;
								vaux[lead] = bufferFBc1ed[lead][TAMBFB - 1];
								pMaxFBc1ed[lead] = TAMBFB - 1;
								for (j = TAMBFB - 2; j >= 0; j--) {
									if (bufferFBc1ed[lead][j] > vaux[lead]) {
										v2aux[lead] = vaux[lead];
										vaux[lead] = bufferFBc1ed[lead][j];
										p2MaxFBc1ed[lead] = pMaxFBc1ed[lead];
										pMaxFBc1ed[lead] = j;
									} else if (bufferFBc1ed[lead][j] > v2aux[lead]) {
										v2aux[lead] = bufferFBc1ed[lead][j];
										p2MaxFBc1ed[lead] = j;
									}
								}
							} else if (min1E[lead] < bufferFBc1ed[lead][p2MaxFBc1ed[lead]]) {
								pMaxFBc1ed[lead] = p2MaxFBc1ed[lead];
								p2MaxFBc1ed[lead] = -1;
							}
						}
					} else if (min1E[lead] >= bufferFBc1ed[lead][pMaxFBc1ed[lead]]) {
						p2MaxFBc1ed[lead] = pMaxFBc1ed[lead];
						pMaxFBc1ed[lead] = aFBc1x[lead];
					} else if (aFBc1x[lead] == p2MaxFBc1ed[lead]) {
						if (min1E[lead] < bufferFBc1ed[lead][p2MaxFBc1ed[lead]])
							p2MaxFBc1ed[lead] = -1;
					} else if (p2MaxFBc1ed[lead] != -1 && min1E[lead] >= bufferFBc1ed[lead][p2MaxFBc1ed[lead]])
						p2MaxFBc1ed[lead] = aFBc1x[lead];

					bufferFBc1ed[lead][aFBc1x[lead]] = min1E[lead];

					if (aFBc1x[lead] == pMinFBc1de[lead]) {
						if (max1D[lead] > bufferFBc1de[lead][pMinFBc1de[lead]]) {
							if (p2MinFBc1de[lead] == -1) {
								bufferFBc1de[lead][aFBc1x[lead]] = max1D[lead];

								v2aux[lead] = INT16_MAX;
								vaux[lead] = bufferFBc1de[lead][TAMBFB - 1];
								pMinFBc1de[lead] = TAMBFB - 1;
								for (j = TAMBFB - 2; j >= 0; j--) {
									if (bufferFBc1de[lead][j] < vaux[lead]) {
										v2aux[lead] = vaux[lead];
										vaux[lead] = bufferFBc1de[lead][j];
										p2MinFBc1de[lead] = pMinFBc1de[lead];
										pMinFBc1de[lead] = j;
									} else if (bufferFBc1de[lead][j] < v2aux[lead]) {
										v2aux[lead] = bufferFBc1de[lead][j];
										p2MinFBc1de[lead] = j;
									}
								}
							} else if (max1D[lead] > bufferFBc1de[lead][p2MinFBc1de[lead]]) {
								pMinFBc1de[lead] = p2MinFBc1de[lead];
								p2MinFBc1de[lead] = -1;
							}
						}
					} else if (max1D[lead] <= bufferFBc1de[lead][pMinFBc1de[lead]]) {
						p2MinFBc1de[lead] = pMinFBc1de[lead];
						pMinFBc1de[lead] = aFBc1x[lead];
					} else if (aFBc1x[lead] == p2MinFBc1de[lead]) {
						if (max1D[lead] > bufferFBc1de[lead][p2MinFBc1de[lead]])
							p2MinFBc1de[lead] = -1;
					} else if (p2MinFBc1de[lead] != -1 && max1D[lead] <= bufferFBc1de[lead][p2MinFBc1de[lead]])
						p2MinFBc1de[lead] = aFBc1x[lead];

					bufferFBc1de[lead][aFBc1x[lead]] = max1D[lead];

					if (aFBc1x[lead] != TAMBFB - 1)
						(aFBc1x[lead])++;
					else
						aFBc1x[lead] = 0;

					storeOutput((bufferFBc1ed[lead][pMaxFBc1ed[lead]] + bufferFBc1de[lead][pMinFBc1de[lead]]) >> 1, lead);
				}

			//Si no esta lleno el bufferDBc[lead]
			//Llenamos el bufferEBc[lead] con los valores de DBc en la diferencia
			} else if (aBDBc[lead] <= TAMBcm) {

				bufferFBc1[lead][aBFBc1[lead]] = bufferEnt[lead][rBEnt[lead]] - bufferDBc[lead][aBDBc[lead] - 1];

				if (rBEnt[lead] != TAMBEnt - 1)
					(rBEnt[lead])++;
				else
					rBEnt[lead] = 0;

				if (aBFBc1[lead] != TAMBFB - 1)
					(aBFBc1[lead])++;
				else
					aBFBc1[lead] = 0;

				if (bufferDBc[lead][aBDBc[lead] - 1] <= bufferEBc[lead][pMinEBc[lead]]) {
					p2MinEBc[lead] = pMinEBc[lead];
					pMinEBc[lead] = aBEBc[lead];
				} else if (p2MinEBc[lead] != -1 && bufferDBc[lead][aBDBc[lead] - 1] <= bufferEBc[lead][p2MinEBc[lead]])
					p2MinEBc[lead] = aBEBc[lead];

				bufferEBc[lead][aBEBc[lead]] = bufferDBc[lead][aBDBc[lead] - 1];
				(aBEBc[lead])++;

				j = indiceB1[lead];
				if (indiceB1[lead] != TAMB1*(TAMB1-1))
					indiceB1[lead] += TAMB1;
				else
					indiceB1[lead] = 0;

				min1E[lead] = bufferFBc1[lead][0] - B1[j];
				max1D[lead] = bufferFBc1[lead][0] + B1[j];

				for(component=1; component<TAMB1; component++)
				{
					min1E[lead] = min1E[lead]>bufferFBc1[lead][component]-B1[j+component]? bufferFBc1[lead][component] - B1[j + component] : min1E[lead];
					max1D[lead] = max1D[lead]<bufferFBc1[lead][component]+B1[j+component]? bufferFBc1[lead][component] + B1[j + component] : max1D[lead];
				}

				//Paso de valores y mover puntero de bufferFBc1ed
				if (aFBc1x[lead] == pMaxFBc1ed[lead]) {
					if (min1E[lead] < bufferFBc1ed[lead][pMaxFBc1ed[lead]]) {
						if (p2MaxFBc1ed[lead] == -1) {
							bufferFBc1ed[lead][aFBc1x[lead]] = min1E[lead];

							//vaux[lead] = bufferFBc1ed[lead][0][k];
							v2aux[lead] = INT16_MIN;
							//pMaxFBc1ed[lead][k] = 0;
							//for (j = 1; j < TAMBFB; j++) {
							vaux[lead] = bufferFBc1ed[lead][TAMBFB - 1];
							pMaxFBc1ed[lead] = TAMBFB - 1;
							for (j = TAMBFB - 2; j >= 0; j--) {
								if (bufferFBc1ed[lead][j] > vaux[lead]) {
									v2aux[lead] = vaux[lead];
									vaux[lead] = bufferFBc1ed[lead][j];
									p2MaxFBc1ed[lead] = pMaxFBc1ed[lead];
									pMaxFBc1ed[lead] = j;
								} else if (bufferFBc1ed[lead][j] > v2aux[lead]) {
									v2aux[lead] = bufferFBc1ed[lead][j];
									p2MaxFBc1ed[lead] = j;
								}
							}
						} else if (min1E[lead] < bufferFBc1ed[lead][p2MaxFBc1ed[lead]]) {
							pMaxFBc1ed[lead] = p2MaxFBc1ed[lead];
							p2MaxFBc1ed[lead] = -1;
						}
					}
				} else if (min1E[lead] >= bufferFBc1ed[lead][pMaxFBc1ed[lead]]) {
					p2MaxFBc1ed[lead] = pMaxFBc1ed[lead];
					pMaxFBc1ed[lead] = aFBc1x[lead];
				} else if (aFBc1x[lead] == p2MaxFBc1ed[lead]) {
					if (min1E[lead] < bufferFBc1ed[lead][p2MaxFBc1ed[lead]])
						p2MaxFBc1ed[lead] = -1;
				} else if (p2MaxFBc1ed[lead] != -1 && min1E[lead] >= bufferFBc1ed[lead][p2MaxFBc1ed[lead]])
					p2MaxFBc1ed[lead] = aFBc1x[lead];

				bufferFBc1ed[lead][aFBc1x[lead]] = min1E[lead];

				if (aFBc1x[lead] == pMinFBc1de[lead]) {
					if (max1D[lead] > bufferFBc1de[lead][pMinFBc1de[lead]]) {
						if (p2MinFBc1de[lead] == -1) {
							bufferFBc1de[lead][aFBc1x[lead]] = max1D[lead];

							v2aux[lead] = INT16_MAX;
							vaux[lead] = bufferFBc1de[lead][TAMBFB - 1];
							pMinFBc1de[lead] = TAMBFB - 1;
							for (j = TAMBFB - 2; j >= 0; j--) {
								if (bufferFBc1de[lead][j] < vaux[lead]) {
									v2aux[lead] = vaux[lead];
									vaux[lead] = bufferFBc1de[lead][j];
									p2MinFBc1de[lead] = pMinFBc1de[lead];
									pMinFBc1de[lead] = j;
								} else if (bufferFBc1de[lead][j] < v2aux[lead]) {
									v2aux[lead] = bufferFBc1de[lead][j];
									p2MinFBc1de[lead] = j;
								}
							}
						} else if (max1D[lead] > bufferFBc1de[lead][p2MinFBc1de[lead]]) {
							pMinFBc1de[lead] = p2MinFBc1de[lead];
							p2MinFBc1de[lead] = -1;
						}
					}
				} else if (max1D[lead] <= bufferFBc1de[lead][pMinFBc1de[lead]]) {
					p2MinFBc1de[lead] = pMinFBc1de[lead];
					pMinFBc1de[lead] = aFBc1x[lead];
				} else if (aFBc1x[lead] == p2MinFBc1de[lead]) {
					if (max1D[lead] > bufferFBc1de[lead][p2MinFBc1de[lead]])
						p2MinFBc1de[lead] = -1;
				} else if (p2MinFBc1de[lead] != -1 && max1D[lead] <= bufferFBc1de[lead][p2MinFBc1de[lead]])
					p2MinFBc1de[lead] = aFBc1x[lead];

				bufferFBc1de[lead][aFBc1x[lead]] = max1D[lead];

				if (aFBc1x[lead] != TAMBFB - 1)
					(aFBc1x[lead])++;
				else
					aFBc1x[lead] = 0;

				storeOutput((bufferFBc1ed[lead][pMaxFBc1ed[lead]] + bufferFBc1de[lead][pMinFBc1de[lead]]) >> 1, lead);
			}
		}
	}//while
 }






