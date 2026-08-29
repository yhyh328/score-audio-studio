import { isPositiveInteger } from "./numberPredicates";
									
const MAX_TIME_SIGNATURE_NUMERATOR = 30;

export function isValidTimeSignature(numerator: number, denominator: number): boolean {									
	const validNumerator : boolean = isPositiveInteger(numerator) &&
                                    numerator <= MAX_TIME_SIGNATURE_NUMERATOR;
	const validDenominator: boolean = denominator === 1 ||	
                                       denominator === 2 ||
                                       denominator === 4 ||
                                       denominator === 8 ||
                                       denominator === 16;
	return (validNumerator && validDenominator);								
}
