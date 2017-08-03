#pragma once

#include "Partition.hpp"
#include "logging/Logger.hpp"

namespace precice {
namespace partition {


/**
 * @brief A partition that is provided by the participant.
 *
 * The participant already provides a partition by calling setMeshVertices etc.
 * If required the mesh needs to be sent to another participant.
 * Furthermore, distribution data structures need to be set up.
 */
class ProvidedPartition : public Partition
{
public:

   /**
    * @brief Constructor
    */
   ProvidedPartition (bool hasToSend);

   virtual ~ProvidedPartition() {}

   /**
    * @brief The mesh is gathered and sent to another participant (if required)
    */
   virtual void communicate ();

   /**
    * @brief All distribution data structures are set up.
    */
   virtual void compute ();

private:

   static logging::Logger _log;

   bool _hasToSend;

};

}} // namespace precice, partition
