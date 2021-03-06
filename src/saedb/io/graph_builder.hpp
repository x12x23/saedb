#pragma once
#include <cstdint>
#include <cstddef>
#include <map>
#include <vector>
#include <utility>

namespace sae {
namespace io {

    typedef std::uint64_t vid_t;
    typedef std::uint64_t eid_t;
    using std::size_t;
    using std::map;
    using std::vector;
    using std::make_pair;

    enum GraphStorageType {
        MemoryMappedGraph = 0,
        GraphStorageTypeCount
    };

    template<class edge_data_t>
    struct edge_with_data {
        vid_t source;
        vid_t target;
        edge_data_t data;
    };

    struct GraphWriter {
        virtual void AppendVertex(void* data) = 0;
        virtual void AppendEdge(vid_t source, vid_t target, void* data) = 0;
        virtual void Close() = 0;
        virtual ~GraphWriter(){}
    };

    GraphWriter* CreateMemoryMappedGraphWriter(const char * prefix, vid_t n, eid_t m, uint32_t vdata_size, uint32_t edata_size);

    template<typename vkey_t, class vertex_data_t, class edge_data_t>
    struct GraphBuilder
    {
    public:

        virtual void AddVertex(vkey_t key, vertex_data_t data) {
            auto id = map(key);
            vertices[id] = data;
        }

        virtual void AddEdge(vkey_t source, vkey_t target, edge_data_t data) {
            auto sid = map(source);
            auto tid = map(target);
            edge_with_data<edge_data_t> e {sid, tid, data};
            edges.push_back(e);
        }

        virtual vid_t VertexCount() {
            return vertices.size();
        }

        virtual eid_t EdgeCount() {
            return edges.size();
        }

        virtual bool Save(const char * prefix, GraphStorageType type = MemoryMappedGraph) {
            if (type == MemoryMappedGraph) {
                GraphWriter* writer = CreateMemoryMappedGraphWriter(prefix, vertices.size(), edges.size(), sizeof(vertex_data_t), sizeof(edge_data_t));
                if (!writer) return false;
                for (auto& v : vertices) {
                    writer->AppendVertex(&v);
                }
                for (auto& e: edges) {
                    writer->AppendEdge(e.source, e.target, &e.data);
                }
                writer->Close();
                delete writer;
            }
        }

    private:
        std::map<vkey_t, uint64_t> vid_map;
        std::vector<vertex_data_t> vertices;
        std::vector<edge_with_data<edge_data_t>> edges;

        vid_t map(vkey_t key) {
            vid_t result;
            auto it = vid_map.find(key);
            if (it == vid_map.end()) {
                result = vertices.size();
                vertices.resize(result + 1);
                vid_map.insert(make_pair(key, result));
            } else {
                result = it->second;
            }
            return result;
        }
    };

}
}
