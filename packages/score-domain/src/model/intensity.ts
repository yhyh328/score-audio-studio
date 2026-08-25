/**
 * Sustained dynamic level.
 * Transient dynamic markings such as sfz/fp will be modeled separately in a later phase.
 */
export type Dynamic = "ppp" | "pp" | "p" | "mp"| "mf" | "f" | "ff" | "fff" 			
						
type DynamicVelocityMap = Record<Dynamic, number>;						
						
export const DEFAULT_DYNAMIC_VELOCITY_MAP: 						
	Readonly<DynamicVelocityMap> = {					
		ppp:  20,				
		pp :  32,				
		p  :  44,				
		mp :  58,				
		mf :  72,				
		f  :  88,				
		ff : 104,				
		fff: 120,				
};						

/**
 * Intensity of a note event.
 *
 * 'dynamic' represents the musical dynamic marking shown in the score.
 *
 * If 'velocity' is omitted, the system resolves the velocity from
 * the default dynamic-to-velocity mapping.
 *
 * If 'velocity' is specified, it overrides the default velocity
 * while preserving the dynamic marking.
 */
export interface Intensity {
  dynamic: Dynamic;
  velocity?: number;
}			