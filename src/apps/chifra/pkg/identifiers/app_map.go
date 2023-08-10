package identifiers

import (
	"strings"

	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/types"
)

type ResolvedId struct {
	BlockNumber      uint64
	TransactionIndex uint64
	Original         string
}

func (r *ResolvedId) String() string {
	return r.Original
}

// AsMap takes command line identifiers for blocks or transactions and returns a map of appearances to allocated
// pointers to SimpleTransactions. The map is keyed by the appearance and the value is the allocated pointer.
// We don't know what type of identifier we have until we try to resolve it.
func AsMap(chain string, ids []Identifier) (map[ResolvedId]*types.SimpleTransaction, int, error) {
	ret := make(map[ResolvedId]*types.SimpleTransaction)
	for index, rng := range ids {
		if rawIds, err := rng.ResolveTxs(chain); err != nil {
			if blockIds, err := rng.ResolveBlocks(chain); err != nil {
				return nil, 0, err
			} else {
				for _, raw := range blockIds {
					s := ResolvedId{
						BlockNumber: uint64(raw),
						Original:    strings.Replace(ids[index].Orig, "-", ".", -1),
					}
					ret[s] = new(types.SimpleTransaction)
				}
			}
		} else {
			for _, raw := range rawIds {
				s := ResolvedId{
					BlockNumber:      uint64(raw.BlockNumber),
					TransactionIndex: uint64(raw.TransactionIndex),
					Original:         strings.Replace(ids[index].Orig, "-", ".", -1),
				}
				ret[s] = new(types.SimpleTransaction)
			}
		}
	}

	return ret, len(ret), nil
}
