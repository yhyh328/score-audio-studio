import { isPositiveInteger } from "./numberPredicates";
									
export function isValidTimeSignature(numerator: number, denominator: number): boolean {									
	const validNumerator : boolean = isPositiveInteger(numerator);								
	const validDenominator: boolean = denominator === 1 ||	
                                       denominator === 2 ||
                                       denominator === 4 ||
                                       denominator === 8 ||
                                       denominator === 16;
	return (validNumerator && validDenominator);								
}
